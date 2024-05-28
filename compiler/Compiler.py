"""Module implements communication with compilers."""
import os
import asyncio
from asyncio.subprocess import Process
from typing import Dict, List, Set, TypedDict, AsyncGenerator

from pydantic.dataclasses import dataclass
from aiopath import AsyncPath
from aiofile import async_open
from compiler.types.ide_types import SupportedCompilers
from compiler.config import LIBRARY_PATH, BUILD_DIRECTORY
from compiler.types.inner_types import CommandResult, BuildFile, File


def get_file_extension(suffixes: List[str]) -> str:
    """Create string extension from Path.suffixes."""
    return ''.join(suffixes).replace('.', '', 1)


async def get_build_files(
        project_path: AsyncPath) -> AsyncGenerator[BuildFile, None]:
    """
    Get all files from 'build' directory in project path direcrory.

    Create build directory if doesn't exist..
    """
    project_build_directory = project_path.joinpath('./build')
    await project_path.mkdir(exist_ok=True)
    async for path in project_build_directory.rglob('*'):
        if not path.is_file():
            continue
        async with async_open(path, 'rb') as f:
            file_data = await f.read()
            extension = get_file_extension(path.suffixes)
            filename = str(path.relative_to(
                project_build_directory)).split('.')[0]
            yield BuildFile(filename=filename,
                            extension=extension,
                            fileContent=file_data)


async def create_project(project_path: AsyncPath,
                         source_files: List[File]) -> None:
    """Save source file into project directory, create project directory\
        if it doesn't exist."""
    await project_path.mkdir(parents=True, exist_ok=True)
    for source_file in source_files:
        file_path = project_path.joinpath(
            f'{source_file.filename}.'
            f'{source_file.extension}')
        async with async_open(file_path, 'wb') as f:
            await f.write(source_file.fileContent)


async def run_commands(project_directory: AsyncPath,
                       source_files: List[File],
                       config_commands: List[str]
                       ) -> AsyncGenerator[CommandResult, None]:
    """
    Save source files to project directory and run build\
        commands one by one.

    Create project directory if it doesn't exist.
    Create project_directory/build directory if it doesn't exist.
    """
    await create_project(project_directory, source_files)
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
        yield CommandResult(command=command,
                            return_code=process.returncode,
                            stdout=stdout,
                            stderr=stderr)


class CompilerException(Exception):
    """Errors dureing compiling."""

    ...


@dataclass
class CompilerResult:
    """Result of compiling Process."""

    return_code: int
    stdout: str
    stderr: str


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
        return f'{LIBRARY_PATH}{platform}/'

    @staticmethod
    async def getBuildFiles(
            libraries: Set[str],
            compiler: str,
            directory: str,
            platform: str) -> Set[str]:
        """Get set of libraries path, thats need to compile."""
        build_files: Set[str] = set()
        for glob in Compiler.supported_compilers[compiler]['extension']:
            async for file in AsyncPath(directory).glob(glob):
                build_files.add(file.name)

        match compiler:
            # get compiled object files
            case 'gcc' | 'g++':
                for library in libraries:
                    build_files.add(
                        ''.join(
                            [
                                '../',
                                Compiler._path(platform),
                                '/build/',
                                library,
                                '.o'
                            ]
                        )
                    )
            case _:
                ...
        return build_files

    @staticmethod
    async def compile_project(
        base_dir: str,
        flags: List[str],
        compiler: str
    ) -> CompilerResult:
        """Compile project in base_dir by compiler with flags."""
        process: Process = await asyncio.create_subprocess_exec(
            compiler,
            *flags,
            cwd=base_dir,
            text=False,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        await process.wait()
        stdout, stderr = await process.communicate()
        if process.returncode is None:
            raise CompilerException('Process doesnt return code.')

        return CompilerResult(process.returncode,
                              str(stdout.decode('utf-8')),
                              str(stderr.decode('utf-8')))

    @staticmethod
    async def compile(base_dir: str,
                      build_files: Set[str],
                      flags: List[str],
                      compiler: SupportedCompilers) -> CompilerResult:
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
        await process.wait()
        stdout, stderr = await process.communicate()

        if process.returncode is None:
            raise CompilerException('Process doesnt return code.')

        return CompilerResult(process.returncode,
                              str(stdout.decode('utf-8')),
                              str(stderr.decode('utf-8')))

    @staticmethod
    async def include_source_files(platform_id: str,
                                   libraries: Set[str],
                                   target_directory: str
                                   ) -> None:
        """Include source files from platform's \
            library directory to target directory."""
        path = os.path.join(LIBRARY_PATH, f'{platform_id}/')
        path_to_libs = set([os.path.join(path, library)
                           for library in libraries])
        await asyncio.create_subprocess_exec('cp',
                                             *path_to_libs,
                                             target_directory,
                                             cwd=BUILD_DIRECTORY)

    @staticmethod
    async def includeLibraryFiles(
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
                                             cwd=BUILD_DIRECTORY)
