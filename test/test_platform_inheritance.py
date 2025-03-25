"""Test platform inheritance functions."""
from contextlib import asynccontextmanager
import os
from typing import Any, Callable, Dict, List, Awaitable
import json

import aiofile
import pytest
from aiopath import AsyncPath
from compiler.platform_manager import PlatformException, PlatformManager
from compiler.types.inner_types import File
from compiler.types.platform_types import Platform, SetupFunction
from compiler.platform_handler import _delete_platform

pytest_plugins = ('pytest_asyncio',)

test_platforms_path = ('test/test_platform_inheritance'
                       '/test_resolve_dependencies/')
test_resolve_components_path = ('test/test_platform_inheritance/'
                                'test_resolve_components'
                                )


@asynccontextmanager
async def add_platforms(platforms: List[Platform],
                        source_files: List[List[File]],
                        images: List[List[File]],
                        autodelete: bool = True,
                        with_resolving: bool = False,):
    """Add platform to PlatformManager. Delete platform after\
        "with" statement.

    with_resolving - resolve platform dependencies
    autodelete - delete platform after with
    """
    platform_manager = PlatformManager()
    try:
        for i, platform in enumerate(platforms):
            new_platforms_info = (
                await platform_manager.add_platform(
                    platform,
                    source_files[i],
                    images[i])
            )

            if with_resolving:
                deps = await platform_manager._resolve_dependencies(platform)
                new_platforms_info[platform.id].dependencies = deps
            platform_manager.platforms_info = new_platforms_info
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
    # TODO: resolve versions
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


@pytest.fixture
async def platforms_with_components():
    """Get test platforms with components."""
    files = ['with_button', 'with_led', 'with_serial']
    platforms: List[Platform] = []
    for file in files:
        with open(os.path.join(test_resolve_components_path, f'{file}.json'),
                  'r') as f:
            platform = Platform(**json.load(f))
            platforms.append(platform)

    return platforms


@pytest.mark.asyncio
async def test_resolve_components(platforms_with_components: List[Platform]):
    """Test resolving components."""
    platform_manager = PlatformManager()
    async with add_platforms(
            platforms_with_components,
            [[]for _ in range(len(platforms_with_components))],
            [[] for _ in range(len(platforms_with_components))], True, True
    ) as platforms:
        platform = platforms[-1]
        assert platform_manager.get_resolved_component(
            platform, 'LED') is not None
        assert platform_manager.get_resolved_component(
            platform, 'Button'
        ) is not None
        assert platform_manager.get_resolved_component(
            platform, 'Serial'
        ) is not None


@pytest.fixture
async def source_files(
    request: pytest.FixtureRequest,
) -> Dict[str, List[File]]:
    """Получить исходные файлы платформ из указанных папок."""
    source_files: Dict[str, List[File]] = {}
    parameters: tuple[List[str], List[str]] = request.param
    platform_ids, paths = parameters
    for i, path in enumerate(paths):
        async for file in AsyncPath(path).rglob('*'):
            source_files[platform_ids[i]] = []
            if await file.is_file():
                async with aiofile.async_open(file, 'r') as f:
                    source_files[platform_ids[i]].append(
                        File(
                            fileContent=await f.read(),
                            filename=file.name.split('.')[0],
                            extension=''.join(file.suffixes)[1:]
                        )
                    )

    return source_files


@pytest.mark.parametrize('source_files',
                         [
                             [
                                 ['with_serial', 'with_button', 'with_led'],
                                 [
                                     os.path.join(
                                         test_resolve_components_path,
                                         'source/with_serial/'
                                     ),
                                     os.path.join(
                                         test_resolve_components_path,
                                         'source/with_button/'
                                     ),
                                     os.path.join(
                                         test_resolve_components_path,
                                         'source/with_led/')
                                 ]
                             ]
                         ],
                         indirect=True
                         )
@pytest.mark.asyncio
async def test_resolve_file(source_files: Dict[str, List[File]],
                            platforms_with_components: List[Platform]):
    """Тестирование получения файла из родительских платформ."""
    platform_manager = PlatformManager()
    async with add_platforms(
            platforms_with_components,
            [
                source_files['with_button'],
                source_files['with_led'],
                source_files['with_serial']
            ],
            [[], [], []],
            with_resolving=True
    ) as platforms:
        platform = platforms[-1]
        assert await platform_manager.get_resolved_file(
            platform, 'Button.c'
        ) is not None  # У дальнего предка
        assert await platform_manager.get_resolved_file(
            platform,
            'Serial.c'
        ) is not None  # У себя
        assert await platform_manager.get_resolved_file(
            platform,
            'LED.c'
        ) is not None  # У родителя
        assert await platform_manager.get_resolved_file(
            platform,
            'blabla.c'
        ) is None


@pytest.mark.parametrize('resolve_func_name, expected',
                         [
                             pytest.param(
                                 'get_resolved_default_includes',
                                 ['Button.c', 'Serial.c', 'LED.c'],
                                 id='get_resolved_default_includes'
                             ),
                             pytest.param(
                                 'get_resolved_default_build_files',
                                 ['Button.c', 'Serial.c',
                                  'LED.c', 'SerialHelper.c'],
                                 id='get_resolved_default_build_files'
                             ),
                             pytest.param(
                                 'get_resolved_default_setup_functions',
                                 [
                                     SetupFunction(
                                         functionName='initButtons', args=[]),
                                     SetupFunction(
                                         functionName='initLed', args=[]),
                                     SetupFunction(
                                         functionName='initSerial', args=[]
                                     ),
                                     SetupFunction(
                                         functionName='initBaud', args=[])
                                 ],
                                 id='get_resolved_default_setup_functions'
                             )
                         ])
@pytest.mark.asyncio
async def test_resolve_list(platforms_with_components: List[Platform],
                            resolve_func_name: str,
                            expected: List[str]):
    """Тест сбора единого списка значений с учетом наследования."""
    platform_manager = PlatformManager()
    async with add_platforms(
            platforms_with_components,
            [[]
             for _ in range(len(platforms_with_components))],
            [[] for _ in range(len(platforms_with_components))],
            with_resolving=True) as platforms:
        platform = platforms[-1]
        resolve_func: Callable[[Platform], Awaitable[List[Any]]] = getattr(
            platform_manager, resolve_func_name)
        assert await resolve_func(platform) == expected
