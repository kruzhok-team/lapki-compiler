import json
import base64
import aiohttp
import time
from typing import Optional, Set
from datetime import datetime
from aiohttp import web
from aiofile import async_open
from aiopath import AsyncPath

from compiler.types.ide_types import CompilerSettings
from compiler.types.inner_types import CompilerResponse, File


try:
    from .fullgraphmlparser.stateclasses import StateMachine
    from .types.ws_types import Message
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
    from .fullgraphmlparser.stateclasses import StateMachine
    from compiler.types.ws_types import Message
    from compiler.types.ide_types import IdeStateMachine
    from compiler.GraphmlParser import GraphmlParser
    from compiler.CJsonParser import CJsonParser
    from compiler.fullgraphmlparser.graphml_to_cpp import CppFileWriter
    from compiler.Compiler import Compiler
    from compiler.JsonConverter import JsonConverter
    from compiler.RequestError import RequestError
    from compiler.config import BUILD_DIRECTORY, MAX_MSG_SIZE
    from compiler.Logger import Logger


class HandlerException(Exception):
    ...


class Handler:
    def __init__(self):
        pass

    @staticmethod
    async def readSourceFile(filename: str, extension: str, path: str) -> File:
        async with async_open(f'{path}{filename}.{extension}', 'r') as f:
            data = await f.read()
        return File(
            filename=filename,
            extension=extension,
            fileContent=data
        )

    @staticmethod
    async def main(request: web.Request) -> web.WebSocketResponse:
        ws = web.WebSocketResponse(autoclose=False, max_msg_size=MAX_MSG_SIZE)
        await ws.prepare(request)
        await Logger.logger.info(request)
        async for msg in ws:
            await Logger.logger.info(msg)
            if msg.type == aiohttp.WSMsgType.TEXT:
                processed_msg = msg.data
                match processed_msg:
                    case 'close':
                        await ws.close()
                    case 'arduino':
                        await Handler.handle_ws_compile(request, ws)
                    case 'berlogaImport':
                        await Handler.handle_berloga_import(request, ws)
                    case 'berlogaExport':
                        await Handler.handle_berloga_export(request, ws)
                    case _:
                        await ws.send_str(f'Unknown {msg}! Use close, arduino, berlogaImport, berlogaExport')
            elif msg.type == aiohttp.WSMsgType.ERROR:
                pass

        return ws

    @staticmethod
    async def handle_ws_compile(request: web.Request, ws: Optional[web.WebSocketResponse] = None):
        if ws is None:
            ws = web.WebSocketResponse(
                autoclose=False, max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)
        try:
            await Logger.logger.info(request)
            data = IdeStateMachine(**await ws.receive_json())
            await Logger.logger.info(data)
            compiler_settings: CompilerSettings | None = data.compilerSettings
            if compiler_settings is None:
                raise Exception()
            compiler = compiler_settings.compiler
            flags = compiler_settings.flags
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
                    build_files = await Compiler.getBuildFiles(libraries=libraries, compiler=compiler, directory=path, platform=platform)
                    await Compiler.includeLibraryFiles(libraries, dirname, '.h', platform)
                    await Logger.logger.info(f'{libraries} included')
                case 'arduino-cli':
                    platform = 'ino'
                    dirname += 'sketch/'
                    path += 'sketch/'
                    await AsyncPath(path).mkdir(parents=True)
                    sm = parser.parseStateMachine(data)
                    await CppFileWriter(sm).write_to_file(path, 'ino')
                    await Logger.logger.info('Parsed and wrote to ino')
                    libraries = libraries.union(
                        libraries, Compiler.c_default_libraries)
                    build_files = await Compiler.getBuildFiles(libraries=libraries, compiler=compiler, directory=path, platform=platform)
                    await Compiler.includeLibraryFiles(libraries, dirname, '.h', platform)
                    await Compiler.includeLibraryFiles(libraries, dirname, '.ino', platform)
                    await Compiler.includeLibraryFiles(
                        Compiler.c_default_libraries,
                        dirname,
                        '.c',
                        platform)
                    await Logger.logger.info(f'{libraries} included')
                case _:
                    await Logger.logger.info(f'Unsupported compiler {compiler}')
                    await RequestError(f'Unsupported compiler {compiler}. \
                        Supported compilers: {Compiler.supported_compilers.keys()}').dropConnection(ws)
                    return ws

            result = await Compiler.compile(base_dir=path, build_files=build_files, flags=flags, compiler=compiler)
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
                            response.binary.append(File(
                                filename=path.name.split('.')[0],
                                extension=''.join(path.suffixes),
                                fileContent=b64_data.decode('ascii'),
                            ))

                response.source.append(await Handler.readSourceFile('sketch', extension, source_path))
                response.source.append(await Handler.readSourceFile('sketch', 'h', source_path))
            await Logger.logger.info(response)
            await ws.send_json(response)
        except KeyError as e:
            await Logger.logger.error(f'Invalid request, there isn\'t {e.args[0]} key.')
            await RequestError(f'Invalid request, there isn\'t {e.args[0]} key.').dropConnection(ws)
            await ws.close()
            return ws
        except Exception as e:
            await Logger.logException()
            await RequestError('Something went wrong').dropConnection(ws)
            await ws.close()
            return ws
        return ws

    @staticmethod
    async def handle_ws_compile_source(request):  # type: ignore
        ws = web.WebSocketResponse(max_msg_size=MAX_MSG_SIZE)
        await ws.prepare(request)
        data = json.loads(await ws.receive_json())
        await Logger.logger.info(data)
        try:
            source = data['source']
            flags = data['compilerSettings']['flags']
            compiler = data['compilerSettings']['compiler']
        except KeyError as e:
            await RequestError(f'Invalid request, there isn\'t key {e.args[0]}').dropConnection(ws)
        if compiler not in Compiler.supported_compilers:
            await Logger.logger.error(f'Unsupported compiler {compiler}.')
            await RequestError(f'Unsupported compiler {compiler}.\
                Supported compilers: {Compiler.supported_compilers.keys()}').dropConnection(ws)

        dirname = BUILD_DIRECTORY + str(datetime.now()) + '/'
        parser = CJsonParser()
        if compiler == 'arduino-cli':
            dirname += source[0]['filename'] + '/'

        await AsyncPath(dirname).mkdir(parents=True, exist_ok=True)
        files = parser.getFiles(source)
        for file in files:
            path = ''.join([dirname, file.name, file.extension])
            async with async_open(path, 'w') as f:
                await f.write(file.content)
        if compiler in ['g++', 'gcc']:
            platform = 'cpp'
        else:
            platform = 'arduino'
        build_files = await Compiler.getBuildFiles(libraries=[], compiler=compiler, directory=dirname, platform=platform)
        result = await Compiler.compile(base_dir=dirname, build_files=build_files, flags=flags, compiler=compiler)
        response = {
            'result': 'OK',
            'return code': result.return_code,
            'stdout': result.stdout,
            'stderr': result.stderr,
            'binary': [],
        }

        async for path in AsyncPath(''.join([dirname, '/build/'])).rglob('*'):
            if await path.is_file():
                async with async_open(path, 'rb') as f:
                    binary = await f.read()
                    fileinfo = {}
                    fileinfo['filename'] = path.name
                    b64_data = base64.b64encode(binary)
                    fileinfo['fileContent'] = b64_data.decode('ascii')
                    response['binary'].append(fileinfo)

        await ws.send_json(response)
        await ws.close()

        return ws

    @staticmethod
    def calculateBearlogaId() -> str:
        return f'{(time.time() + 62135596800) * 10000000:f}'.split('.')[0]

    @staticmethod
    async def handle_berloga_import(request, ws=None):
        if ws is None:
            ws = web.WebSocketResponse(max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)
        unprocessed_xml = await ws.receive_str()
        filename_without_extension = await ws.receive_str()

        subplatform = filename_without_extension.split('_')[0]

        await Logger.logger.info('XML received!')
        try:
            response = await GraphmlParser.parse(unprocessed_xml, platform=f'BearlogaDefend-{subplatform}')
            await Logger.logger.info('Converted!')
            await ws.send_json(
                {
                    'result': 'OK',
                    'stdout': '',
                    'stderr': '',
                    'source': [{
                        'filename': f'{subplatform}_{Handler.calculateBearlogaId()}',
                        'extension': '.json',
                        'fileContent': response
                    }],
                    'binary': []
                })
        except KeyError as e:
            await Logger.logException()
            await RequestError(f'There isn\'t key {e.args[0]}').dropConnection(ws)
        except Exception as e:
            await Logger.logException()
            await RequestError('Something went wrong!').dropConnection(ws)

        return ws

    @staticmethod
    async def handle_berloga_export(request, ws: Optional[web.WebSocketResponse] = None):
        if ws is None:
            ws = web.WebSocketResponse(max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)
        data = IdeStateMachine(**await ws.receive_str())
        filename = await ws.receive_str()
        await Logger.logger.info(data)
        try:

            parser = CJsonParser()
            sm = parser.parseStateMachine(data)
            states_with_id = {}
            for state in sm.states:
                states_with_id[state.name] = state

            converter = JsonConverter(ws)

            xml = await converter.parse(states_with_id, data.initialState)

            await ws.send_json(
                {
                    'filename': f'{filename}_{Handler.calculateBearlogaId()}',
                    'extension': 'graphml',
                    'fileContent': xml
                })
            await Logger.logger.info('Converted!')
        except KeyError as e:
            await Logger.logException()
            await RequestError(f'There isn\'t key {e.args[0]}').dropConnection(ws)
            return ws
        except Exception as e:
            await Logger.logException()
            await RequestError(f'Something went wrong {e.args[0]}').dropConnection(ws)
            return ws
        return ws

    @staticmethod
    async def handle_get_compile(request):
        return web.Response(text='Hello world!')
