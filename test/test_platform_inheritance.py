"""Test platform inheritance functions."""
from contextlib import asynccontextmanager
import os
from typing import List
import json

import pytest
from compiler.platform_manager import PlatformException, PlatformManager
from compiler.types.inner_types import File
from compiler.types.platform_types import Platform
from compiler.platform_handler import _delete_platform

pytest_plugins = ('pytest_asyncio',)

test_platforms_path = ('test/test_platform_inheritance'
                       '/test_resolve_dependencies/')


@asynccontextmanager
async def add_platforms(platforms: List[Platform],
                        source_files: List[List[File]],
                        images: List[List[File]],
                        autodelete: bool = True):
    """Add platform to PlatformManager. Delete platform after\
        "with" statement."""
    platform_manager = PlatformManager()
    try:
        for i, platform in enumerate(platforms):
            platform_manager.platforms_info = (
                await platform_manager.add_platform(
                    platform,
                    source_files[i],
                    images[i])
            )

        yield platforms
    finally:
        if autodelete:
            for platform in platforms:
                await _delete_platform(platform.id)


@pytest.fixture
async def empty_platforms() -> List[Platform]:
    """Read empty test platforms from test_platforms_path directory."""
    platform_ids = ['empty0', 'empty1',
                    'empty4', 'empty2', 'empty5', 'empty3', 'empty6']
    platforms: List[Platform] = []
    for platform_id in platform_ids[::-1]:
        with open(os.path.join(test_platforms_path, f'{platform_id}.json'),
                  'r') as f:
            platform = Platform(**json.load(f))
            platforms.append(platform)
    return platforms


def read_similar_platforms(name: str, start: int, end: int) -> List[Platform]:
    """Read blob of platforms with similar name."""
    platforms: List[Platform] = []

    for i in range(start, end):
        with open(os.path.join(test_platforms_path, f'{name}{i}.json'),
                  'r') as f:
            platform = Platform(**json.load(f))
            platforms.append(platform)

    return platforms


@pytest.fixture
async def circular_platforms() -> List[Platform]:
    """Read circular test platforms from test_platforms_path directory."""
    return read_similar_platforms('circular', 0, 3)


@pytest.mark.asyncio
async def test_resolve_dependencies(empty_platforms: List[Platform],
                                    circular_platforms: List[Platform]):
    """Test _resolve_dependencies function."""
    platform_manager = PlatformManager()
    async with add_platforms(empty_platforms,
                             [[] for _ in range(len(empty_platforms))],
                             [[] for _ in range(len(empty_platforms))]
                             ) as platforms:
        platform = platforms[-1]
        deps = await platform_manager._resolve_dependencies(platform)
        deps_platform_id = [
            'empty1', 'empty4', 'empty2', 'empty5', 'empty3', 'empty6'
        ]
        assert [dep.id for dep in deps] == deps_platform_id

    with pytest.raises(PlatformException):
        platform = await platform_manager.load_platform(
            f'{test_platforms_path}/self_inherit.json')

        await platform_manager._resolve_dependencies(platform)

    with pytest.raises(PlatformException):
        async with add_platforms(circular_platforms,
                                 [[] for _ in range(len(circular_platforms))],
                                 [[] for _ in range(len(circular_platforms))]
                                 ) as platforms:
            await platform_manager._resolve_dependencies(
                circular_platforms[0])
