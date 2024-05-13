"""Module implements handling and processing requests."""
import asyncio
import json
import base64
import os
import time
import os.path
from typing import List, Optional, Set, get_args, AsyncGenerator
from datetime import datetime
from itertools import chain

import aiohttp
from aiohttp import web
from aiofile import async_open
from aiopath import AsyncPath
from pydantic import ValidationError
from compiler.config import MODULE_PATH
from compiler.CGML import parse, CGMLException
from compiler.Compiler import CompilerResult
from compiler.types.inner_types import (
    CompilerResponse,
    File,
    CompileCommands,
    CommandResult
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

BinaryFile = File


def _get_project_directory() -> str:
    """Generate path to project directory, but don't create it."""
    base_dir = str(datetime.now()) + '/'
    base_dir = base_dir.replace(' ', '_')
    return os.path.join(MODULE_PATH, base_dir)


async def _create_project(project_path: AsyncPath,
                          source_files: List[File]) -> None:
    """Save source file into project directory, create project directory\
        if it doesn't exist."""
    await project_path.mkdir(parents=True, exist_ok=True)
    for source_file in source_files:
        file_path = project_path.joinpath(source_file.filename)
        async with async_open(file_path, 'wb') as f:
            await f.write(source_file.fileContent)


async def _raw_compile(project_directory: AsyncPath,
                       source_files: List[File],
                       config_commands: List[str]
                       ) -> AsyncGenerator[CommandResult, None]:
    """
    Save source files to project directory and run build\
        commands one by one.

    Create project directory if it doesn't exist.
    Create project_directory/build directory if it doesn't exist.
    """
    await _create_project(project_directory, source_files)
    build_path = project_directory.joinpath('./build')
    await build_path.mkdir(parents=True, exist_ok=True)
    for command in config_commands:
        process = await asyncio.create_subprocess_exec(
            command,
            cwd=project_directory,
            text=False,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE)
        stdout, stderr = await process.communicate()
        yield CommandResult(command, process.returncode, stdout, stderr)


async def _get_build_files(project_directory: str):
    ...


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
    build_path = os.path.join(base_dir, 'build/')
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


class CompileCommandException(Exception):
    """Got unknown compile command."""

    ...


class HandlerException(Exception):
    """Errors during processing requests."""

    ...


class Handler:
    """Class for processing requests."""

    def __init__(self):
        pass

    @staticmethod
    async def readSourceFile(
            filename: str, extension: str, path: str) -> File:
        """Read file by path."""
        async with async_open(f'{path}{filename}.{extension}', 'r') as f:
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
                BUILD_DIRECTORY, base_dir.replace(' ', '_'), 'sketch/')
            await AsyncPath(base_dir).mkdir(parents=True)
            compiler_result: CompilerResult = await compile_xml(xml, base_dir)
            response = await create_response(base_dir, compiler_result)
            await Logger.logger.info(response)
            await ws.send_json(response.model_dump())
        except CGMLException as e:
            await Logger.logException()
            await RequestError(', '.join(map(str, e.args))).dropConnection(ws)
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
    async def handle_ws_raw_compile(
            request: web.Request,
            ws: Optional[web.WebSocketResponse] = None):
        """
        Handle for compiling from source.

        Send: CompilerResponse | RequestError
        """
        if ws is None:
            ws = web.WebSocketResponse(
                autoclose=False, max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)

        source_files: List[File] = []
        build_commands: List[str] = []
        current_file = File('', '', bytes())
        finished = False
        try:
            async for msg in ws:
                if finished:
                    break
                if msg.type == aiohttp.WSMsgType.TEXT:
                    match msg.data:
                        case 'file_path':
                            current_file.filename = await ws.receive_str()
                            break
                        case 'file_content':
                            current_file.fileContent = await ws.receive_bytes()
                            source_files.append(current_file)
                            current_file = File('', '', bytes())
                            break
                        case 'build_command':
                            build_command = await ws.receive_str()
                            for command in get_args(CompileCommands):
                                if command in build_command:
                                    break
                            else:
                                raise CompileCommandException(
                                    f'Unknown compile command {build_command}.'
                                    'Compile command must contains'
                                    'one of these words: '
                                    f'{get_args(CompileCommands)}.'
                                )
                            build_commands.append(await ws.receive_str())
                            break
                        case 'end':
                            finished = True
                            break
                project_directory = AsyncPath(_get_project_directory())
                command_result_generator = _raw_compile(
                    project_directory, source_files, build_commands)
                async for command_result in command_result_generator:
                    await ws.send_json(command_result)
        except CompileCommandException as e:
            await ws.send_str(str(e))
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
