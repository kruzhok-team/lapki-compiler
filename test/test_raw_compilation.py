"""Module implements testing raw compilation module."""
import os
from typing import List

import aioshutil
import pytest
from aiopath import AsyncPath
from compiler.config import get_config
from compiler.types.inner_types import File
from compiler.Compiler import (
    get_build_files,
    get_filename,
    get_file_extension,
    run_commands
)

PATH_TO_PROJECT = os.path.join(
    f'{get_config().module_directory}',
    '../test/test_raw_compilation_project/')

pytest_plugins = ('pytest_asyncio',)


@pytest.fixture
def commands() -> List[str]:
    """Commands to compile test project."""
    return ['make', 'make build']


@pytest.fixture
async def project_files() -> List[File]:
    """Read files from PATH_TO_PROJECT."""
    path = AsyncPath(PATH_TO_PROJECT)
    files: List[File] = []
    async for path in path.rglob('*'):
        if await path.is_file():
            file_content = await path.read_bytes()
            files.append(
                File(filename=get_filename(str(path.relative_to(
                    PATH_TO_PROJECT))),
                    extension=get_file_extension(path.suffixes),
                    fileContent=file_content)
            )

    return files


async def test_raw_compile(project_files: List[File], commands: List[str]):
    """Compile test project."""
    commands_generator = run_commands(
        AsyncPath(PATH_TO_PROJECT), project_files, commands)
    async for command_result in commands_generator:
        assert command_result.return_code == 0
    build_files_generator = get_build_files(AsyncPath(PATH_TO_PROJECT))
    i = 0
    async for _ in build_files_generator:
        i += 1
    await aioshutil.rmtree(os.path.join(PATH_TO_PROJECT, './build'))
    assert i != 0
