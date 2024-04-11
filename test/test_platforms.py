import json
from typing import List

import pytest
from compiler.types.platform_types import Platform
from compiler.platform_handler import _add_platform
from compiler.types.inner_types import File

pytest_plugins = ('pytest_asyncio',)


@pytest.fixture
def source_files() -> List[File]:
    """Get test source files."""
    return [
        File('Test', 'cpp', 'ooooo;')
    ]


@pytest.fixture
def load_platform():
    """Load Autoborder platform."""
    with open('compiler/platforms/Autoborder-new.json', 'r') as f:
        data = json.load(f)
    return Platform(**data)


def test_not_compile_platform_creation(load_platform):
    ...


@pytest.mark.asyncio
async def test_add_platform(load_platform, source_files):
    platform_id = await _add_platform(load_platform, source_files)
    # TODO: remove platform
