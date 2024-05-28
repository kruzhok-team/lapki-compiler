from typing import List
import pytest
from aiopath import AsyncPath

from compiler.types.inner_types import File

PATH_TO_PROJECT = 'test/test_raw_compilation_project'

pytest_plugins = ('pytest_asyncio',)


@pytest.fixture
async def project_files() -> List[File]:
    path = AsyncPath(PATH_TO_PROJECT)
    files: List[File] = []
    async for path in path.rglob('*'):
        if await path.is_file():
            file_content = await path.read_bytes()
            File(filename=)


def test_raw_compile():
    path = AsyncPath(PATH_TO_PROJECT)
    files = []
    # async for path in path.rglob('*'):
    # if await path.is_file():
    # ...
