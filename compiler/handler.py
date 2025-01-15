"""Module implements handling and processing requests."""
import json
import base64
import os
import time
import os.path
from typing import Dict, List, Optional, Set
from datetime import datetime
from itertools import chain

from aiohttp import web
from aiofile import async_open
from aiopath import AsyncPath
from pydantic import ValidationError
from compiler.CGML import parse, CGMLException
from compiler.platform_manager import PlatformException
from compiler.types.inner_types import (
    CompilerResponse,
    File,
    CommandResult,
    LegacyResponse,
    StateMachineResult
)
from compiler.types.ide_types import CompilerSettings
from compiler.fullgraphmlparser.stateclasses import (
    StateMachine,
    SMCompilingSettings
)
from compiler.types.ide_types import IdeStateMachine
from compiler.graphml_parser import GraphmlParser
from compiler.cjson_parser import CJsonParser
from compiler.fullgraphmlparser.graphml_to_cpp import CppFileWriter
from compiler.Compiler import Compiler
from compiler.json_converter import JsonConverter
from compiler.request_error import RequestError
from compiler.config import get_config
from compiler.logger import Logger

BinaryFile = File


def get_sm_path(base_directory: str,
                sm_id: str) -> str:
    """Get str path to state machine project."""
    return os.path.join(base_directory, sm_id, 'sketch/')


async def create_response(
        base_dir: str,
        compiler_result: Dict[str, tuple[List[CommandResult], StateMachine]]
) -> CompilerResponse:
    """
    Get source files, binary files from\
        directory and create CompilerResponse.

        Doesn't send anything.
    """
    status = 'OK'

    compiler_response = CompilerResponse(
        result=status,
        state_machines={}
    )

    build_path = os.path.join(base_dir, 'build/')

    for sm_id, commands_result_and_sm in compiler_result.items():
        commands_result, sm = commands_result_and_sm
        path_to_sm = get_sm_path(base_dir, sm_id)
        sm_compile_status = 'OK'
        for command in commands_result:
            if (command.return_code):
                sm_compile_status = 'NOTOK'
                compiler_response.result = 'NOTOK'
                break
        response = StateMachineResult(
            name=sm_id,
            result=sm_compile_status,
            commands=commands_result,
            binary=[],
            source=[]
        )
        build_path = os.path.join(path_to_sm, 'build/')
        async for path in AsyncPath(build_path).rglob('*'):
            if await path.is_file():
                async with async_open(path, 'rb') as f:
                    binary = await f.read()
                    b64_data: bytes = base64.b64encode(binary)
                    response.binary.append(
                        File(
                            filename=path.name.split('.')[0],
                            extension=''.join(path.suffixes),
                            fileContent=b64_data.decode('ascii'),
                        )
                    )
        response.source.append(await Handler.readSourceFile(
            'sketch',
            sm.main_file_extension,
            path_to_sm)
        )
        response.source.append(await Handler.readSourceFile(
            'sketch',
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


async def compile_xml(
    xml: str,
    base_dir_path: str) -> Dict[str, tuple[List[CommandResult],
                                           StateMachine]]:
    """
    Compile CGML scheme.

    This function generate code from scheme, compile it.

    Doesn't send anything.
    """
    state_machines: Dict[str, StateMachine] = await parse(xml)
    compile_results: Dict[str, tuple[List[CommandResult], StateMachine]] = {}
    default_library = get_default_libraries()
    for sm_id, sm in state_machines.items():
        path = await create_sm_directory(base_dir_path, sm_id)
        await CppFileWriter(sm, True, True).write_to_file(
            path,
            sm.main_file_extension)
        settings: SMCompilingSettings | None = sm.compiling_settings
        build_path = os.path.join(path, 'build/')
        await AsyncPath(build_path).mkdir(exist_ok=True)
        if settings is None:
            raise PlatformException(
                'У платформы отсутствуют настройки компиляции.')
        await Compiler.include_source_files(Compiler.DEFAULT_LIBRARY_ID,
                                            '1.0',  # TODO: Версия стандарта?
                                            default_library,
                                            path)
        await Compiler.include_source_files(settings.platform_id,
                                            settings.platform_version,
                                            settings.build_files,
                                            path)

        commands_results = await Compiler.compile_project(
            path,
            settings.platform_compiler_settings
        )

        compile_results[sm_id] = (commands_results, sm)

    return compile_results


class HandlerException(Exception):
    """Errors during processing requests."""

    ...


class Handler:
    """Class for processing requests."""

    @staticmethod
    async def readSourceFile(
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
        config = get_config()
        if ws is None:
            ws = web.WebSocketResponse(
                autoclose=False, max_msg_size=config.max_msg_size)
            await ws.prepare(request)
        try:
            xml = await ws.receive_str()
            base_dir = os.path.join(
                config.build_directory,
                str(datetime.now()).replace(' ', '_'),
                'sketch'
            )
            await AsyncPath(base_dir).mkdir(parents=True)
            compiler_result = await compile_xml(
                xml,
                base_dir
            )
            response = await create_response(base_dir,
                                             compiler_result,
                                             )
            await Logger.logger.info(response)
            await ws.send_json(response.model_dump())
        except CGMLException as e:
            await Logger.logException()
            await RequestError(e.error_data.message).dropConnection(
                ws,
                sm_id=e.error_data.sm_id
            )
        except Exception:
            await Logger.logException()
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
        config = get_config()
        if ws is None:
            ws = web.WebSocketResponse(
                autoclose=False, max_msg_size=config.max_msg_size)
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
            path = os.path.join(config.build_directory, dirname)
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
                    ino_libraries = {f'{library}.ino'
                                     for library in libraries}
                    h_libraries = {f'{library}.h'
                                   for library in libraries}
                    libraries = h_libraries | ino_libraries
                    await Compiler.include_source_files(platform,
                                                        '1.0',
                                                        libraries,
                                                        path)
                    await Compiler.include_source_files(
                        Compiler.DEFAULT_LIBRARY_ID,
                        '1.0',
                        get_default_libraries(),
                        path)
                    await Logger.logger.info(f'{libraries} included')

            result: CommandResult = await Compiler.compile(
                path,
                set(),
                ['compile', *flags],
                compiler)
            response = LegacyResponse(
                result='NOTOK',
                return_code=result.return_code if result.return_code else 0,
                stdout=str(result.stdout),
                stderr=str(result.stderr),
                binary=[],
                source=[]
            )

            if result.return_code == 0:
                response.result = 'OK'
                build_path = ''.join(
                    [config.build_directory, dirname, 'build/'])
                source_path = ''.join([config.build_directory, dirname])
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

                response.source.append(await Handler.readSourceFile(
                    'sketch',
                    extension,
                    source_path)
                )
                response.source.append(await Handler.readSourceFile(
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
                               f'{e.args[0]} key.').dropConnection(ws,
                                                                   legacy=True)
            await ws.close()
        except ValidationError as e:
            await Logger.logger.info(e.errors())
            await RequestError(
                f'Validation error: {e.errors()}'
            ).dropConnection(ws, True)
        except Exception:
            await Logger.logException()
            await RequestError('Something went wrong').dropConnection(
                ws,
                legacy=True
            )
            await ws.close()
        return ws

    @staticmethod
    def calculateBearlogaId() -> str:
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
        config = get_config()
        if ws is None:
            ws = web.WebSocketResponse(max_msg_size=config.max_msg_size)
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
                        f'{Handler.calculateBearlogaId()}',
                        'extension': '.json',
                        'fileContent': response
                    }],
                    'binary': []
                })
        except KeyError as e:
            await Logger.logException()
            await RequestError('There isnt'
                               f'key {e.args[0]}').dropConnection(ws,
                                                                  legacy=True
                                                                  )
        except Exception:
            await Logger.logException()
            await RequestError('Something went wrong!').dropConnection(
                ws,
                legacy=True
            )

        return ws

    @staticmethod
    async def handle_berloga_export(
            request: web.Request,
            ws: Optional[web.WebSocketResponse] = None):
        """
        Generate yed-GraphMl from Lapki IDE's internal JSON scheme.

        Send: File | RequestError
        """
        config = get_config()
        if ws is None:
            ws = web.WebSocketResponse(max_msg_size=config.max_msg_size)
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
                    'filename': f'{filename}_{Handler.calculateBearlogaId()}',
                    'extension': 'graphml',
                    'fileContent': xml
                })
            await Logger.logger.info('Converted!')
        except KeyError as e:
            await Logger.logException()
            await RequestError('There isnt'
                               f'key {e.args[0]}').dropConnection(ws,
                                                                  legacy=True)
            return ws
        except Exception as e:
            await Logger.logException()
            await RequestError('Something went wrong'
                               f'{e.args[0]}').dropConnection(ws,
                                                              legacy=True)
            return ws
        return ws
