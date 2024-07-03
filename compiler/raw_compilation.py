"""Module implements raw compilation."""
from typing import List, Optional, get_args

import aiohttp
from aiohttp import web
from aiopath import AsyncPath
from compiler.Compiler import (
    get_build_files,
)
from compiler.utils import get_file_extension, get_project_directory
from compiler.types.inner_types import (
    File,
    CompileCommands
)
from compiler.platform_handler import check_token
from compiler.Compiler import run_commands
from compiler.config import get_config
from compiler.Logger import Logger


class CompileCommandException(Exception):
    """Got unknown compile command."""

    ...


async def handle_ws_raw_compile(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None,
        access_token: Optional[str] = None):
    """
    Handle for compiling from source.

    Send: CompilerResponse | RequestError
    """
    if ws is None:
        config = get_config()
        ws = web.WebSocketResponse(
            autoclose=False, max_msg_size=config.max_msg_size)
        await ws.prepare(request)
    try:
        if access_token is None:
            access_token = await ws.receive_str()
        check_token(access_token)
        source_files: List[File] = []
        build_commands: List[str] = []
        current_file = File(filename='',
                            extension='',
                            fileContent=bytes())
        finished = False
        async for msg in ws:
            if finished:
                break
            if msg.type == aiohttp.WSMsgType.TEXT:
                match msg.data:
                    case 'file_path':
                        current_file.filename = await ws.receive_str()
                        current_file.extension = get_file_extension(
                            AsyncPath(current_file.filename).suffixes)
                        break
                    case 'file_content':
                        current_file.fileContent = await ws.receive_bytes()
                        source_files.append(current_file)
                        current_file = File(filename='',
                                            extension='',
                                            fileContent=bytes())
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
            project_directory = AsyncPath(get_project_directory())
            command_result_generator = run_commands(
                project_directory, source_files, build_commands
            )
            async for command in command_result_generator:
                await ws.send_str('command-result')
                await ws.send_json(command.model_dump())
            await ws.send_str('end-commands')
            build_files_generator = get_build_files(project_directory)
            async for build_file in build_files_generator:
                await ws.send_str('build-file-name')
                await ws.send_str(build_file.filename)
                await ws.send_str('build-file-extension')
                await ws.send_str(build_file.extension)
                await ws.send_str('build-file-content')
                await ws.send_bytes(build_file.fileContent)
            await ws.send_str('end-build-files-send')
    except CompileCommandException as e:
        await Logger.logException()
        await ws.send_str(str(e))
    return ws
