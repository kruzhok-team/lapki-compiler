"""Module to test platform processing."""
import asyncio
import json
import time
from typing import List

import pytest
from aiofile import async_open
from compiler.PlatformManager import (
    PlatformException,
    PlatformManager,
    _get_path_to_platform
)
from compiler.platform_handler import (
    _add_platform,
    _get_platform,
    _update_platform,
    _delete_platform_by_versions
)
from compiler.types.platform_types import Platform, PlatformInfo
from compiler.types.inner_types import InnerFile

pytest_plugins = ('pytest_asyncio',)


@pytest.fixture
def source_files() -> List[InnerFile]:
    """Get test source files."""
    return [
        InnerFile(
            filename='aaaa/Test',
            extension='cpp',
            fileContent='ooooo;')
    ]


@pytest.fixture
def images() -> List[InnerFile]:
    """Get test images."""
    with open('test/test_resources/test_image.jpg', 'rb') as f:
        return [InnerFile(filename='babuleh',
                          extension='jpg',
                          fileContent=f.read())]


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


@pytest.mark.asyncio
async def test_add_platform(load_platform_from_file: Platform,
                            source_files: List[InnerFile],
                            images: List[InnerFile]):
    platform_id = await _add_platform(
        load_platform_from_file,
        source_files,
        images)
    assert PlatformManager.platforms_versions_info == {
        platform_id: PlatformInfo(
            versions=set(['1.0']),
            access_tokens=set()
        )
    }
    # TODO: remove platform


@pytest.mark.asyncio
async def test_get_raw_platform(add_platform: tuple[str, Platform]):
    platform_id, platform = add_platform

    test_result = await _get_platform(platform_id, platform.version)
    async with async_open(
        _get_path_to_platform(platform_id, platform.version)
    ) as f:
        expected = await f.read()
        assert test_result == expected


@pytest.mark.asyncio
async def test_get_platform_sources(add_platform: tuple[str, Platform],
                                    source_files: List[InnerFile]):
    platform_id, platform = add_platform
    source_gen = await PlatformManager.get_platform_sources(
        platform_id, platform.version)
    result_sources: List[InnerFile] = [source async for source in source_gen]
    assert result_sources == source_files


@pytest.mark.asyncio
async def test_get_platform_images(add_platform: tuple[str, Platform],
                                   images: List[InnerFile]):
    platform_id, platform = add_platform
    image_gen = await PlatformManager.get_platform_images(
        platform_id, platform.version)
    result_images: List[InnerFile] = [img async for img in image_gen]
    assert result_images == images


@pytest.mark.asyncio
async def test_update_platform(add_platform: tuple[str, Platform],
                               source_files: List[InnerFile],
                               images: List[InnerFile]):
    platform_id, platform = add_platform
    new_platform = platform.model_copy(deep=True)
    new_platform.version = '2.0'
    # TODO: add test token?
    await _update_platform(new_platform, '', source_files, images)
    assert PlatformManager.platforms_versions_info == {
        platform_id: PlatformInfo(
            versions=set(['1.0', '2.0']),
            access_tokens=set()
        )
    }
    # If platform with this version already exist
    with pytest.raises(PlatformException):
        await _update_platform(new_platform, '', source_files, images)
    # If platform with this id doesn't exist
    with pytest.raises(PlatformException):
        new_platform.id = 'blabla'
        await _update_platform(new_platform, '', source_files, images)


@pytest.mark.asyncio
async def test_delete_platform_by_version(add_platform: tuple[str, Platform]):
    platform_id, platform = add_platform
    await _delete_platform_by_versions(platform_id, platform.version)
    assert PlatformManager.has_version(platform_id, platform.version) is False
    # Не проходится из-за непонятного поведения
    # Если проверить платформы на существование в самой функции
    # PlatformManager.delete_platform_by_versions
    # То выведется True, и словарь platforms_versions_info будет дейтсвительно
    # пустым, но здесь, в тесте, он почему-то все равно имеет ключ platform_id
    assert PlatformManager.platform_exist(platform_id) is False
