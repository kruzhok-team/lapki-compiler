"""Module to test platform processing."""
import json
from typing import List

import pytest
from aiofile import async_open
from compiler.PlatformManager import PlatformManager, _get_path_to_platform
from compiler.types.platform_types import Platform
from compiler.platform_handler import _add_platform, _get_platform
from compiler.types.inner_types import File

pytest_plugins = ('pytest_asyncio',)


@pytest.fixture
def source_files() -> List[File]:
    """Get test source files."""
    return [
        File('aaaa/Test', 'cpp', 'ooooo;')
    ]


@pytest.fixture
def images() -> List[File]:
    """Get test images."""
    with open('test/test_resources/test_image.jpg', 'rb') as f:
        return [File('babuleh', 'jpg', f.read())]


@pytest.fixture
def load_platform_from_file() -> Platform:
    """Load Autoborder platform."""
    with open('compiler/platforms/Autoborder-new.json', 'r') as f:
        data = json.load(f)
    return Platform(**data)


@pytest.mark.asyncio
@pytest.fixture
async def add_platform(load_platform_from_file,
                       images,
                       source_files) -> tuple[str, Platform]:
    platform_id = await _add_platform(
        load_platform_from_file,
        source_files,
        images)
    return platform_id, load_platform_from_file


def test_not_compile_platform_creation(load_platform_from_file: Platform):
    ...


@pytest.mark.asyncio
async def test_add_platform(load_platform_from_file: Platform,
                            source_files: List[File],
                            images: List[File]):
    platform_id = await _add_platform(
        load_platform_from_file,
        source_files,
        images)
    assert dict(PlatformManager.platforms_versions_info) == {
        platform_id: {'1.0'}}
    # TODO: remove platform


@pytest.mark.asyncio
async def test_get_platform(add_platform: tuple[str, Platform]):
    platform_id, platform = add_platform

    test_result = await _get_platform(platform_id, platform.version)
    async with async_open(
        _get_path_to_platform(platform_id, platform.version)
    ) as f:
        expected = await f.read()
        assert test_result == expected
