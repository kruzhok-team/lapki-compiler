"""Module implements handling and processing requests."""
import json
import base64
import time
from typing import List, Optional, Set
from datetime import datetime
from itertools import chain

import aiohttp
from aiohttp import web
from aiofile import async_open
from aiopath import AsyncPath
from pydantic import ValidationError


try:
    from .CGML import parse, CGMLException
    from .Compiler import CompilerResult
    from .types.inner_types import CompilerResponse, InnerFile
    from .types.ide_types import CompilerSettings
    from .fullgraphmlparser.stateclasses import (
        StateMachine,
        SMCompilingSettings
    )
    from .types.ide_types import IdeStateMachine
    from .GraphmlParser import GraphmlParser
    from .CJsonParser import CJsonParser
    from .fullgraphmlparser.graphml_to_cpp import CppFileWriter
    from .Compiler import Compiler
    from .JsonConverter import JsonConverter
    from .RequestError import RequestError
    from .config import BUILD_DIRECTORY, MAX_MSG_SIZE
    from .Logger import Logger
except ImportError:
    from compiler.CGML import parse, CGMLException
    from compiler.Compiler import CompilerResult
    from compiler.types.inner_types import CompilerResponse, InnerFile
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


async def create_response(
        base_dir: str,
        compiler_result: CompilerResult) -> CompilerResponse:
    """
    Get source files, binary files from\
        directory and create CompilerResponse.

        Doesn't send anything.
    """
    response = CompilerResponse(
        result='NOTOK' if compiler_result.return_code != 0 else 'OK',
        return_code=compiler_result.return_code,
        stdout=compiler_result.stdout,
        stderr=compiler_result.stderr,
        binary=[],
        source=[]
    )
    build_path = base_dir + 'build/'
    async for path in AsyncPath(build_path).rglob('*'):
        if await path.is_file():
            async with async_open(path, 'rb') as f:
                binary = await f.read()
                b64_data: bytes = base64.b64encode(binary)
                response.binary.append(InnerFile(
                    filename=path.name.split('.')[0],
                    extension=''.join(path.suffixes),
                    fileContent=b64_data.decode('ascii'),
                ))

    response.source.append(await Handler.readSourceFile(
        'sketch',
        'ino',
        base_dir)
    )
    response.source.append(await Handler.readSourceFile(
        'sketch',
        'h',
        base_dir)
    )
    return response


def get_default_libraries() -> Set[str]:
    """
    Get set of default libraries.

    Return example: 'qhsm.c', 'qhsm.h'
    """
    return set(list(
        chain.from_iterable((f'{library}.c', f'{library}.h')
                            for library in Compiler.c_default_libraries))
               )


async def compile_xml(xml: str, base_dir_path: str) -> CompilerResult:
    """
    Compile CGML scheme.

    This function generate code from scheme, compile it.

    Doesn't send anything.
    """
    sm: StateMachine = await parse(xml)
    await CppFileWriter(sm, True, True).write_to_file(base_dir_path, 'ino')
    settings: SMCompilingSettings | None = sm.compiling_settings
    if settings is None:
        raise Exception('Internal error!')
    default_library = get_default_libraries()
    await Compiler.include_source_files(Compiler.DEFAULT_LIBRARY_ID,
                                        default_library,
                                        base_dir_path)
    await Compiler.include_source_files(settings.platform_id,
                                        settings.build_files,
                                        base_dir_path)
    flags = settings.platform_compiler_settings.flags
    compiler = settings.platform_compiler_settings.compiler
    return await Compiler.compile_project(
        base_dir_path,
        flags,
        compiler
    )


class HandlerException(Exception):
    """Errors during processing requests."""

    ...


class Handler:
    """Class for processing requests."""

    def __init__(self):
        pass

    @staticmethod
    async def readSourceFile(
            filename: str, extension: str, path: str) -> InnerFile:
        """Read file by path."""
        async with async_open(f'{path}{filename}.{extension}', 'r') as f:
            data = await f.read()
        return InnerFile(
            filename=filename,
            extension=extension,
            fileContent=data
        )

    @staticmethod
    async def main(request: web.Request) -> web.WebSocketResponse:
        """Root handler, call other handlers."""
        ws = web.WebSocketResponse(autoclose=False, max_msg_size=MAX_MSG_SIZE)
        await ws.prepare(request)
        await Logger.logger.info(request)
        async for msg in ws:
            await Logger.logger.info(msg)
            if msg.type == aiohttp.WSMsgType.TEXT:
                match msg.data:
                    case 'close':
                        await ws.close()
                    case 'arduino':
                        # Я не понимаю, у pyright какие-то проблемы
                        # с моими асинхронными функциями
                        # type: ignore
                        await Handler.handle_ws_compile(request, ws)
                    case 'berlogaImport':
                        # type: ignore
                        await Handler.handle_berloga_import(request, ws)
                    case 'berlogaExport':
                        await Handler.handle_berloga_export(request, ws)
                    case 'cgml':
                        await Handler.handle_cgml_compile(request, ws)
                    case _:
                        await ws.send_str(f'Unknown {msg}!'
                                          'Use close, arduino,'
                                          'berlogaImport, berlogaExport')
            elif msg.type == aiohttp.WSMsgType.ERROR:
                pass

        return ws

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
            base_dir = base_dir.replace(' ', '_') + '/sketch'
            await AsyncPath(base_dir).mkdir()
            compiler_result: CompilerResult = await compile_xml(xml, base_dir)
            response = await create_response(base_dir, compiler_result)
            await Logger.logger.info(response)
            await ws.send_json(response.model_dump())
        except CGMLException as e:
            await Logger.logException()
            await RequestError(e.args[0]).dropConnection(ws)
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
                    build_files = await Compiler.getBuildFiles(
                        libraries,
                        compiler,
                        path,
                        platform)
                    await Compiler.includeLibraryFiles(
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
                    build_files = await Compiler.getBuildFiles(
                        libraries,
                        compiler,
                        path,
                        platform)
                    await Compiler.includeLibraryFiles(
                        libraries,
                        dirname,
                        '.h',
                        platform)
                    await Compiler.includeLibraryFiles(
                        Compiler.c_default_libraries,
                        dirname,
                        '.h',
                        Compiler.DEFAULT_LIBRARY_ID
                    )
                    await Compiler.includeLibraryFiles(
                        libraries,
                        dirname,
                        '.ino',
                        platform)
                    await Compiler.includeLibraryFiles(
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
            response = CompilerResponse(
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
                            response.binary.append(InnerFile(
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

        dirname = BUILD_DIRECTORY + str(datetime.now()) + '/'
        parser = CJsonParser()
        if compiler == 'arduino-cli':
            dirname += source[0]['filename'] + '/'

        await AsyncPath(dirname).mkdir(parents=True, exist_ok=True)
        files: List[InnerFile] = parser.getFiles(source)
        for file in files:
            path = ''.join([dirname, file.filename, file.extension])
            async with async_open(path, 'w') as f:
                await f.write(file.fileContent)
        if compiler in ['g++', 'gcc']:
            platform = 'cpp'
        else:
            platform = 'arduino'
        build_files: Set[str] = await Compiler.getBuildFiles(
            libraries=set(),
            compiler=compiler,
            directory=dirname,
            platform=platform)
        result: CompilerResult = await Compiler.compile(
            dirname,
            build_files,
            flags,
            compiler)
        response = CompilerResponse(
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
                    response.binary.append(InnerFile(
                        filename=path.name,
                        fileContent=b64_data.decode('ascii'),
                        extension=''.join(path.suffixes),
                    ))

        await ws.send_json(response.model_dump())
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
                        f'{Handler.calculateBearlogaId()}',
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

        Send: InnerFile | RequestError
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
                    'filename': f'{filename}_{Handler.calculateBearlogaId()}',
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
