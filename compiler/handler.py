"""Module implements handling and processing requests."""
import json
import base64
import os
import time
from typing import Dict, List, Optional, Set
from datetime import datetime
from itertools import chain

from aiohttp import web
from aiofile import async_open
from aiopath import AsyncPath
from pydantic import ValidationError
from compiler.CGML import parse, CGMLException
from compiler.Compiler import CompilerResult
from compiler.types.inner_types import (
    CompilerResponse,
    File,
    StateMachineId,
    StateMachineResult
)
from compiler.types.ide_types import CompilerSettings
from compiler.fullgraphmlparser.stateclasses import (
    StateMachine,
    SMCompilingSettings
)
from compiler.types.ide_types import IdeStateMachine
from compiler.GraphmlParser import GraphmlParser
from compiler.CJsonParser import CJsonParser
from compiler.fullgraphmlparser.graphml_to_cpp import CppFileWriter
from compiler.Compiler import Compiler
from compiler.JsonConverter import JsonConverter
from compiler.RequestError import RequestError
from compiler.config import BUILD_DIRECTORY, MAX_MSG_SIZE
from compiler.Logger import Logger


def get_sm_path(base_directory: str,
                sm_id: str) -> StateMachineId:
    """Get str path to state machine project."""
    return os.path.join(base_directory, sm_id)


async def create_response(
        base_dir: str,
        compiler_results: Dict[StateMachineId, CompilerResult]
) -> CompilerResponse:
    """
    Get source files, binary files from\
        directory and create CompilerResponse.

        Doesn't send anything.
    """
    compiler_response = CompilerResponse(
        result='OK',
        state_machines={}
    )
    for sm_id, compiler_result in compiler_results.items():
        path_to_sm = get_sm_path(base_dir, sm_id)
        if compiler_result.return_code == 0:
            result = 'OK'
        else:
            result = 'NOTOK'
            compiler_response.result = 'NOTOK'
        response = StateMachineResult(
            result=result,
            return_code=compiler_result.return_code,
            stdout=compiler_result.stdout,
            stderr=compiler_result.stderr,
            binary=[],
            source=[]
        )
        build_path = os.path.join(path_to_sm, 'build/')
        await AsyncPath(build_path).mkdir(exist_ok=True)
        async for path in AsyncPath(build_path).rglob('*'):
            if await path.is_file():
                async with async_open(path, 'rb') as f:
                    binary = await f.read()
                    b64_data: bytes = base64.b64encode(binary)
                    response.binary.append(File(
                        filename=path.name.split('.')[0],
                        extension=''.join(path.suffixes),
                        fileContent=b64_data.decode('ascii'),
                    ))
        response.source.append(await Handler.read_source_file(
            sm_id,
            'ino',
            path_to_sm)
        )
        response.source.append(await Handler.read_source_file(
            sm_id,
            'h',
            path_to_sm)
        )
        compiler_response.state_machines[sm_id] = response
    return compiler_response


def get_default_libraries() -> Set[str]:
    """
    Get set of default libraries.

    Return example: 'qhsm.c', 'qhsm.h'
    """
    return set(list(
        chain.from_iterable((f'{library}.c', f'{library}.h')
                            for library in Compiler.c_default_libraries))
               )


async def create_sm_directory(base_directory: str,
                              sm_id: str) -> str:
    """Create project directory for state machine\
        relative to base directory."""
    path = get_sm_path(base_directory, sm_id)
    await AsyncPath(path).mkdir(parents=True)
    return path


async def compile_xml(xml: str,
                      base_dir_path: str
                      ) -> Dict[StateMachineId, CompilerResult]:
    """
    Compile CGML scheme.

    This function generate code from scheme, compile it.

    A separate project is created for each state machine.
    Each state machine is compiled separately.

    Doesn't send anything.
    """
    state_machines: Dict[str, StateMachine] = await parse(xml)
    compiler_results: Dict[StateMachineId, CompilerResult] = {}
    for sm_id, state_machine in state_machines.items():
        path = await create_sm_directory(base_dir_path, sm_id)
        await CppFileWriter(state_machine, True, True).write_to_file(
            path, 'ino'
        )
        settings: SMCompilingSettings | None = state_machine.compiling_settings
        if settings is None:
            raise Exception('Internal error! Settings compile is None!')
        default_library = get_default_libraries()
        await Compiler.include_source_files(Compiler.DEFAULT_LIBRARY_ID,
                                            default_library,
                                            path)
        await Compiler.include_source_files(settings.platform_id,
                                            settings.build_files,
                                            path)
        flags = settings.platform_compiler_settings.flags
        compiler = settings.platform_compiler_settings.compiler
        compiler_results[sm_id] = await Compiler.compile_project(
            path,
            flags,
            compiler
        )

    return compiler_results


class HandlerException(Exception):
    """Errors during processing requests."""

    ...


class Handler:
    """Class for processing requests."""

    def __init__(self):
        pass

    @staticmethod
    async def read_source_file(
            filename: str, extension: str, path: str) -> File:
        """Read file by path."""
        async with async_open(os.path.join(path, f'{filename}.{extension}'),
                              'r') as f:
            data = await f.read()
        return File(
            filename=filename,
            extension=extension,
            fileContent=data
        )

    @staticmethod
    async def handle_cgml_compile(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None
    ) -> web.WebSocketResponse:
        """Generate code from CGML-scheme and compile it."""
        if ws is None:
            ws = web.WebSocketResponse(
                autoclose=False, max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)
        try:
            xml = await ws.receive_str()
            base_dir = str(datetime.now()) + '/'
            base_dir = os.path.join(
                BUILD_DIRECTORY, base_dir.replace(' ', '_'))
            await AsyncPath(base_dir).mkdir(parents=True)
            compiler_result: Dict[StateMachineId, CompilerResult] = (
                await compile_xml(xml, base_dir)
            )
            response = await create_response(base_dir, compiler_result)
            await Logger.logger.info(response)
            await ws.send_json(response.model_dump())
        except CGMLException as e:
            await ws.send_str('error')
            await Logger.logException()
            await RequestError(', '.join(map(str, e.args))).dropConnection(ws)
        except Exception:
            await Logger.logException()
            await ws.send_str('error')
            await RequestError('Internal error!').dropConnection(ws)
        return ws

    @staticmethod
    async def handle_ws_compile(
            request: web.Request,
            ws: Optional[web.WebSocketResponse] = None):
        """
        Generate code from Lapki IDE's internal JSON scheme\
            and compile it.

        Send: CompilerResponse | RequestError
        """
        if ws is None:
            ws = web.WebSocketResponse(
                autoclose=False, max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)
        try:
            await Logger.logger.info(request)
            a = await ws.receive_str()
            data = IdeStateMachine(**json.loads(a))
            await Logger.logger.info(data)
            compiler_settings: CompilerSettings | None = data.compilerSettings
            if compiler_settings is None:
                raise HandlerException('Internal error: never is reached.')
            compiler = compiler_settings.compiler
            flags: List[str] = compiler_settings.flags
            dirname = str(datetime.now()) + '/'
            dirname = dirname.replace(' ', '_')
            path = BUILD_DIRECTORY + dirname
            extension = Compiler.supported_compilers[compiler]['extension'][0]
            parser = CJsonParser()
            components = list(data.components.values())
            libraries: Set[str] = parser.getLibraries(components)
            match compiler:
                case 'g++' | 'gcc':
                    platform = 'cpp'
                    await AsyncPath(path).mkdir(parents=True)
                    sm: StateMachine = parser.parseStateMachine(data)
                    await CppFileWriter(sm).write_to_file(path, extension)
                    libraries = libraries.union(
                        libraries, Compiler.c_default_libraries)
                    build_files = await Compiler.get_build_files(
                        libraries,
                        compiler,
                        path,
                        platform)
                    await Compiler.include_library_files(
                        libraries,
                        dirname,
                        '.h',
                        platform
                    )
                    await Logger.logger.info(f'{libraries} included')
                case 'arduino-cli':
                    platform = 'ArduinoUno'
                    dirname += 'sketch/'
                    path += 'sketch/'
                    await AsyncPath(path).mkdir(parents=True)
                    sm = parser.parseStateMachine(data)
                    # type: ignore
                    await CppFileWriter(sm).write_to_file(path, 'ino')
                    await Logger.logger.info('Parsed and wrote to ino')
                    build_files = await Compiler.get_build_files(
                        libraries,
                        compiler,
                        path,
                        platform)
                    await Compiler.include_library_files(
                        libraries,
                        dirname,
                        '.h',
                        platform)
                    await Compiler.include_library_files(
                        Compiler.c_default_libraries,
                        dirname,
                        '.h',
                        Compiler.DEFAULT_LIBRARY_ID
                    )
                    await Compiler.include_library_files(
                        libraries,
                        dirname,
                        '.ino',
                        platform)
                    await Compiler.include_library_files(
                        Compiler.c_default_libraries,
                        dirname,
                        '.c',
                        Compiler.DEFAULT_LIBRARY_ID)
                    await Logger.logger.info(f'{libraries} included')

            result: CompilerResult = await Compiler.compile(
                path,
                build_files,
                ['compile', *flags],
                compiler)
            response = StateMachineResult(
                result='NOTOK',
                return_code=result.return_code,
                stdout=result.stdout,
                stderr=result.stderr,
                binary=[],
                source=[]
            )

            if result.return_code == 0:
                response.result = 'OK'
                build_path = ''.join([BUILD_DIRECTORY, dirname, 'build/'])
                source_path = ''.join([BUILD_DIRECTORY, dirname])
                async for path in AsyncPath(build_path).rglob('*'):
                    if await path.is_file():
                        async with async_open(path, 'rb') as f:
                            binary = await f.read()
                            b64_data: bytes = base64.b64encode(binary)
                            response.binary.append(File(
                                filename=path.name.split('.')[0],
                                extension=''.join(path.suffixes),
                                fileContent=b64_data.decode('ascii'),
                            ))

                response.source.append(await Handler.read_source_file(
                    'sketch',
                    extension,
                    source_path)
                )
                response.source.append(await Handler.read_source_file(
                    'sketch',
                    'h',
                    source_path)
                )
            await Logger.logger.info(response)
            await ws.send_json(response.model_dump())
        except KeyError as e:
            await Logger.logger.error('Invalid request, there isnt'
                                      f'{e.args[0]} key.')
            await RequestError('Invalid request, there isnt'
                               f'{e.args[0]} key.').dropConnection(ws)
            await ws.close()
        except ValidationError as e:
            await Logger.logger.info(e.errors())
            await RequestError(
                f'Validation error: {e.errors()}'
            ).dropConnection(ws)
        except Exception:
            await Logger.logException()
            await RequestError('Something went wrong').dropConnection(ws)
            await ws.close()
        return ws

    @staticmethod
    async def handle_ws_compile_source(request: web.Request):
        """
        Legacy handler for compiling from source.

        Send: CompilerResponse | RequestError
        """
        ws = web.WebSocketResponse(max_msg_size=MAX_MSG_SIZE)
        await ws.prepare(request)
        data = json.loads(await ws.receive_json())
        await Logger.logger.info(data)
        try:
            source = data['source']
            flags = data['compilerSettings']['flags']
            compiler = data['compilerSettings']['compiler']
        except KeyError as e:
            await RequestError('Invalid request there'
                               f' isnt key {e.args[0]}').dropConnection(ws)
            return ws
        if compiler not in Compiler.supported_compilers:
            supported_compilers = list(map(
                str,
                Compiler.supported_compilers.keys())
            )
            await Logger.logger.error(f'Unsupported compiler {compiler}.')
            await RequestError(f'Unsupported compiler {compiler}.'
                               'Supported compilers:'
                               f'{supported_compilers}').dropConnection(ws)

        dirname = os.path.join(BUILD_DIRECTORY, str(datetime.now()), '/')
        parser = CJsonParser()
        if compiler == 'arduino-cli':
            dirname += source[0]['filename'] + '/'

        await AsyncPath(dirname).mkdir(parents=True, exist_ok=True)
        files: List[File] = parser.getFiles(source)
        for file in files:
            path = ''.join([dirname, file.filename, file.extension])
            async with async_open(path, 'w') as f:
                await f.write(file.fileContent)
        if compiler in ['g++', 'gcc']:
            platform = 'cpp'
        else:
            platform = 'arduino'
        build_files: Set[str] = await Compiler.get_build_files(
            libraries=set(),
            compiler=compiler,
            directory=dirname,
            platform=platform)
        result: CompilerResult = await Compiler.compile(
            dirname,
            build_files,
            flags,
            compiler)
        response = StateMachineResult(
            result='OK',
            return_code=result.return_code,
            stdout=result.stdout,
            stderr=result.stderr,
            binary=[],
            source=[]
        )

        async for path in AsyncPath(''.join([dirname, '/build/'])).rglob('*'):
            if await path.is_file():
                async with async_open(path, 'rb') as f:
                    binary = await f.read()
                    b64_data: bytes = base64.b64encode(binary)
                    response.binary.append(File(
                        filename=path.name,
                        fileContent=b64_data.decode('ascii'),
                        extension=''.join(path.suffixes),
                    ))

        await ws.send_json(response.model_dump())
        await ws.close()

        return ws

    @staticmethod
    def calculate_bearloga_id() -> str:
        """Generate unique Id for Bearloga's file."""
        return f'{(time.time() + 62135596800) * 10000000:f}'.split('.')[0]

    @staticmethod
    async def handle_berloga_import(
            request: web.Request,
            ws: Optional[web.WebSocketResponse] = None):
        """
        Generate Lapki IDE's internal JSON scheme from yed-GraphMl.

        Send: CompilerResponse | RequestError
        """
        if ws is None:
            ws = web.WebSocketResponse(max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)
        unprocessed_xml = await ws.receive_str()
        filename_without_extension = await ws.receive_str()

        subplatform = filename_without_extension.split('_')[0]

        await Logger.logger.info('XML received!')
        try:
            response = await GraphmlParser.parse(
                unprocessed_xml,
                f'BearlogaDefend-{subplatform}'
            )
            await Logger.logger.info('Converted!')
            await ws.send_json(
                {
                    'result': 'OK',
                    'stdout': '',
                    'stderr': '',
                    'source': [{
                        'filename': f'{subplatform}_'
                        f'{Handler.calculate_bearloga_id()}',
                        'extension': '.json',
                        'fileContent': response
                    }],
                    'binary': []
                })
        except KeyError as e:
            await Logger.logException()
            await RequestError('There isnt'
                               f'key {e.args[0]}').dropConnection(ws)
        except Exception:
            await Logger.logException()
            await RequestError('Something went wrong!').dropConnection(ws)

        return ws

    @staticmethod
    async def handle_berloga_export(
            request: web.Request,
            ws: Optional[web.WebSocketResponse] = None):
        """
        Generate yed-GraphMl from Lapki IDE's internal JSON scheme.

        Send: File | RequestError
        """
        if ws is None:
            ws = web.WebSocketResponse(max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)
        data = IdeStateMachine(**json.loads(await ws.receive_str()))
        filename = await ws.receive_str()
        await Logger.logger.info(data)
        try:
            parser = CJsonParser()
            sm: StateMachine = parser.parseStateMachine(data)
            states_with_id = {}
            for state in sm.states:
                states_with_id[state.name] = state
            converter = JsonConverter(ws)
            xml: str = await converter.parse(states_with_id, data.initialState)
            await ws.send_json(
                {
                    'filename': (f'{filename}_'
                                 f'{Handler.calculate_bearloga_id()}'),
                    'extension': 'graphml',
                    'fileContent': xml
                })
            await Logger.logger.info('Converted!')
        except KeyError as e:
            await Logger.logException()
            await RequestError('There isnt'
                               f'key {e.args[0]}').dropConnection(ws)
            return ws
        except Exception as e:
            await Logger.logException()
            await RequestError('Something went wrong'
                               f'{e.args[0]}').dropConnection(ws)
            return ws
        return ws
