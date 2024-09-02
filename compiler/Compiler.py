"""Module implements communication with compilers."""
import os
import asyncio
from asyncio.subprocess import Process
from typing import Dict, List, Set, TypedDict, AsyncGenerator

from aiopath import AsyncPath
from aiofile import async_open
from compiler.platform_manager import get_source_path
from compiler.types.ide_types import SupportedCompilers
from compiler.types.platform_types import CompilingSettings
from compiler.config import get_config
from compiler.types.inner_types import CommandResult, BuildFile, File
from compiler.utils import get_file_extension, get_filename


async def get_build_files(
        project_path: AsyncPath) -> AsyncGenerator[BuildFile, None]:
    """
    Get all files from 'build' directory in project path direcrory.

    Create build directory if doesn't exist.

    Function join project_path and ./build.
    """
    project_build_directory = project_path.joinpath('./build')
    await project_path.mkdir(exist_ok=True)
    async for path in project_build_directory.rglob('*'):
        if not await path.is_file():
            continue
        async with async_open(path, 'rb') as f:
            file_data = await f.read()
            extension = get_file_extension(path.suffixes)
            filename = get_filename(str(path.relative_to(
                project_build_directory)))
            yield BuildFile(filename=filename,
                            extension=extension,
                            fileContent=file_data)


async def create_project(project_path: AsyncPath,
                         source_files: List[File]) -> None:
    """Save source file into project directory, create project directory\
        if it doesn't exist."""
    await project_path.mkdir(parents=True, exist_ok=True)
    for source_file in source_files:
        if source_file.extension != '':
            file = f'{source_file.filename}.{source_file.extension}'
        else:
            file = source_file.filename
        file_path = os.path.join(str(project_path), file)
        async with async_open(file_path, 'wb') as f:
            await f.write(source_file.fileContent)


async def run_commands(project_directory: AsyncPath,
                       source_files: List[File],
                       config_commands: List[str]
                       ) -> AsyncGenerator[CommandResult, None]:
    """
    Save source files to project directory and run build\
        commands one by one.

    config_commands example: ['make config', 'make run'].

    Each command is splited by ' ' and converted to the form ['make', 'config']

    Create project directory if it doesn't exist.
    Create project_directory/build directory if it doesn't exist.
    """
    await create_project(project_directory, source_files)
    build_path = project_directory.joinpath('./build')
    await build_path.mkdir(parents=True, exist_ok=True)
    for command in config_commands:
        process = await asyncio.create_subprocess_exec(
            *command.split(' '),
            cwd=project_directory,
            text=False,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE)
        stdout, stderr = await process.communicate()
        yield CommandResult(command=command,
                            return_code=process.returncode,
                            stdout=stdout,
                            stderr=stderr)


class CompilerException(Exception):
    """Errors dureing compiling."""

    ...


class SupportedCompiler(TypedDict):
    """Dict with information about awaialable flags and extensions."""

    extension: List[str]
    flags: List[str]


class Compiler:
    """Class for compiling, copying libraries sources."""

    DEFAULT_LIBRARY_ID = 'default'
    c_default_libraries = set(['qhsm'])  # legacy
    supported_compilers: Dict[str, SupportedCompiler] = {
        'gcc': {
            'extension': ['.c', '.cpp'],
            'flags': ['-c', '-std=', '-Wall']},
        'g++': {
            'extension': ['.cpp', '.c'],
            'flags': ['-c', '-std=', '-Wall']},
        'arduino-cli': {
            'extension': ['ino'],
            'flags': ['-b', 'avr:arduino:uno']
        }
    }

    @staticmethod
    def _path(platform: str) -> str:
        return f'{get_config().library_path}{platform}/'

    @staticmethod
    async def compile_project(
        base_dir: str,
        commands: List[CompilingSettings]
    ) -> List[CommandResult]:
        """Compile project in base_dir by compiler with flags."""
        command_results: List[CommandResult] = []
        for command in commands:
            process: Process = await asyncio.create_subprocess_exec(
                command.command,
                *command.flags,
                cwd=base_dir,
                text=False,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )
            await process.wait()
            stdout, stderr = await process.communicate()
            if process.returncode is None:
                raise CompilerException('Process doesnt return code.')
            command_results.append(CommandResult(
                command=command.command + ' ' + ' '.join(command.flags),
                return_code=process.returncode,
                stdout=str(stdout.decode('utf-8')),
                stderr=str(stderr.decode('utf-8'))))
        return command_results

    @staticmethod
    async def compile(base_dir: str,
                      build_files: Set[str],
                      flags: List[str],
                      compiler: SupportedCompilers) -> CommandResult:
        """(Legacy, use compile_project) Run compiler with choosen settings."""
        match compiler:
            case 'g++' | 'gcc':
                await AsyncPath(base_dir + 'build/').mkdir(parents=True,
                                                           exist_ok=True)
                flags.append('-o')
                flags.append('./build/a.out')
                process: Process = await asyncio.create_subprocess_exec(
                    compiler,
                    *build_files,
                    *flags,
                    cwd=base_dir,
                    text=False,
                    stdout=asyncio.subprocess.PIPE,
                    stderr=asyncio.subprocess.PIPE
                )
            case 'arduino-cli':
                process = await asyncio.create_subprocess_exec(
                    compiler,
                    *flags,
                    '--export-binaries',
                    *build_files,
                    cwd=base_dir,
                    text=False,
                    stdout=asyncio.subprocess.PIPE,
                    stderr=asyncio.subprocess.PIPE)
            case _:
                raise CompilerException('Not supported compiler')
        await process.wait()
        stdout, stderr = await process.communicate()

        if process.returncode is None:
            raise CompilerException('Process doesnt return code.')

        return CommandResult(command='compile project',
                             return_code=process.returncode,
                             stdout=str(stdout.decode('utf-8')),
                             stderr=str(stderr.decode('utf-8')))

    @staticmethod
    async def include_source_files(platform_id: str,
                                   platform_version: str,
                                   libraries: Set[str],
                                   target_directory: str
                                   ) -> None:
        """Include source files from platform's \
            library directory to target directory."""
        config = get_config()
        path = get_source_path(platform_id, platform_version)
        path_to_libs = set([os.path.join(path, library)
                           for library in libraries])
        await asyncio.create_subprocess_exec('cp',
                                             *path_to_libs,
                                             target_directory,
                                             cwd=config.build_directory)

    @staticmethod
    async def include_library_files(
            libraries: Set[str],
            target_directory: str,
            extension: str,
            platform: str) -> None:
        """(Legacy, use include_source_files) \
            Функция, которая копирует все необходимые файлы библиотек."""
        paths_to_libs = [''.join(
            [
                f'{Compiler._path(platform)}',
                library,
                extension]
        ) for library in libraries]
        await asyncio.create_subprocess_exec('cp',
                                             *paths_to_libs,
                                             target_directory,
                                             cwd=get_config().build_directory)
