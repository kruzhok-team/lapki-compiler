"""Module implements communication with compilers."""
import os
import asyncio
from asyncio.subprocess import Process
from typing import Dict, List, Set, TypedDict

from pydantic.dataclasses import dataclass
from aiopath import AsyncPath
from compiler.types.ide_types import SupportedCompilers
from compiler.config import LIBRARY_PATH, BUILD_DIRECTORY


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
