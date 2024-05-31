"""Module to test platform processing."""
import json
from contextlib import asynccontextmanager
from typing import List

import pytest
from aiofile import async_open
from compiler.PlatformManager import (
    PlatformException,
    PlatformManager,
    _get_path_to_platform,
)
from compiler.platform_handler import (
    _add_platform,
    _get_platform,
    _update_platform,
    _delete_platform_by_versions,
    _delete_platform,
    _check_token
)
from compiler.access_controller import (
    AccessController,
    AccessControllerException
)
from compiler.types.platform_types import Platform, PlatformMeta
from compiler.types.inner_types import File

pytest_plugins = ('pytest_asyncio',)


@pytest.fixture
def access_controller() -> AccessController:
    return AccessController()


@pytest.fixture
def source_files() -> List[File]:
    """Get test source files."""
    return [
        File(
            filename='aaaa/Test',
            extension='cpp',
            fileContent='ooooo;')
    ]


@pytest.fixture
def platform_manager() -> PlatformManager:
    """Return PlatformManager instance."""
    return PlatformManager()


@pytest.fixture
def images() -> List[File]:
    """Get test images."""
    with open('test/test_resources/test_image.jpg', 'rb') as f:
        return [File(filename='babuleh',
                     extension='jpg',
                     fileContent=f.read())]


@pytest.fixture
def platform() -> Platform:
    """Load Autoborder platform."""
    with open('compiler/platforms/BearlogaDefend-Autoborder/1.0/BearlogaDefend-Autoborder-1.0.json', 'r') as f:
        data = json.load(f)
    return Platform(**data)


@asynccontextmanager
async def add_platform(platform: Platform,
                       source_files: List[File],
                       images: List[File],
                       autodelete: bool = True):
    try:
        platform_id = await _add_platform(
            platform,
            source_files,
            images)
        yield platform_id
    finally:
        if autodelete:
            await _delete_platform(platform.id)


@pytest.mark.asyncio
async def test_add_platform(platform_manager: PlatformManager,
                            platform: Platform,
                            source_files: List[File],
                            images: List[File]):
    platform_id = await _add_platform(
        platform,
        source_files,
        images)
    assert platform_manager.versions_info == {
        platform_id: PlatformMeta(
            versions=set(['1.0']),
            name='Берлога/Защита пасеки - Автобортник',
            author='Lapki TEAM'
        )
    }
    await _delete_platform(platform_id)


@pytest.mark.asyncio
async def test_get_raw_platform(platform: Platform,
                                source_files: List[File],
                                images: List[File]):

    async with add_platform(platform, source_files, images) as platform_id:
        test_result = await _get_platform(platform_id, platform.version)
        async with async_open(
            _get_path_to_platform(platform_id, platform.version)
        ) as f:
            expected = await f.read()
            assert test_result == expected


@pytest.mark.asyncio
async def test_get_platform_sources(platform_manager: PlatformManager,
                                    platform: Platform,
                                    source_files: List[File]):
    async with add_platform(platform, source_files, []) as platform_id:
        source_gen = await platform_manager.get_platform_sources(
            platform_id, platform.version)
        result_sources: List[File] = [source async
                                      for source in source_gen]
        assert result_sources == source_files


@pytest.mark.asyncio
async def test_get_platform_images(platform_manager: PlatformManager,
                                   platform: Platform,
                                   images: List[File]):
    async with add_platform(platform, [], images) as platform_id:
        image_gen = await platform_manager.get_platform_images(
            platform_id, platform.version)
        result_images: List[File] = [img async for img in image_gen]
        assert result_images == images


@pytest.mark.asyncio
async def test_update_platform(platform_manager: PlatformManager,
                               platform: Platform,
                               source_files: List[File],
                               images: List[File]):
    async with add_platform(platform, source_files, images) as platform_id:
        new_platform = platform.model_copy(deep=True)
        new_platform.version = '2.0'
        new_platform.author = 'test'
        new_platform.name = 'test'
        await _update_platform(new_platform, source_files, images)
        assert platform_manager.versions_info == {
            platform_id: PlatformMeta(
                versions=set(['1.0', '2.0']),
                author=new_platform.author,
                name=new_platform.name
            )
        }
        # If platform with this version already exist
        with pytest.raises(PlatformException):
            await _update_platform(new_platform, source_files, images)
        # If platform with this id doesn't exist
        with pytest.raises(PlatformException):
            new_platform.id = 'blabla'
            await _update_platform(new_platform, source_files, images)


@pytest.mark.asyncio
async def test_delete_platform_by_version(platform_manager: PlatformManager,
                                          platform: Platform,
                                          source_files: List[File],
                                          images: List[File]):
    async with (add_platform(platform, source_files, images, False)
                as platform_id):
        await _delete_platform_by_versions(platform_id, platform.version)
        assert platform_manager.platform_exist(platform_id) is False
        with pytest.raises(KeyError):
            platform_manager.has_version(platform_id, platform.version)
    async with (add_platform(platform, source_files, images, autodelete=True)
                as platform_id):
        new_version_platform = platform.model_copy(deep=True)
        new_version_platform.version = '2.0'
        assert platform_manager.versions_info[platform_id].versions == set(
            ('1.0',))


@pytest.mark.asyncio
async def test_delete_platfrom(
    platform_manager: PlatformManager,
    platform: Platform,
    source_files: List[File],
    images: List[File]
):
    async with (add_platform(platform, source_files, images, False)
                as platform_id):
        new_version = platform.model_copy(deep=True)
        new_version.version = '2.0'
        await _update_platform(new_version, [], [])
        await _delete_platform(platform_id)
        assert platform_manager.versions_info == {}


@pytest.mark.asyncio
async def test_check_token(access_controller: AccessController) -> None:
    token = await access_controller.create_token()
    _check_token(token)
    with pytest.raises(AccessControllerException):
        _check_token('blabla')
    
