"""Test platform inheritance functions."""
import os
from typing import List

import pytest
from compiler.platform_manager import PlatformException, PlatformManager
from compiler.types.platform_types import Platform

pytest_plugins = ('pytest_asyncio',)

path_to_empty_platforms = ('test/test_platform_inheritance'
                           '/test_resolve_dependencies/')


@pytest.fixture
async def empty_platforms() -> List[Platform]:
    """Load test platforms from path_to_empty_platforms directory."""
    platform_manager = PlatformManager()
    platforms: List[Platform] = []
    for i in range(0, 7):
        platforms.append(
            await platform_manager.load_platform(
                os.path.join(path_to_empty_platforms, f'empty{i}.json')
            )
        )

    return platforms


@pytest.mark.asyncio
async def test_resolve_dependencies(empty_platforms: List[Platform]):
    """Test _resolve_dependencies function."""
    platform_manager = PlatformManager()
    platform = empty_platforms[0]
    deps = await platform_manager._resolve_dependencies(platform)
    deps_platform_id = ['empty1',
                        'empty4', 'empty2', 'empty5', 'empty3', 'empty6']
    assert [dep.id for dep in deps] == deps_platform_id

    with pytest.raises(PlatformException):
        ...
