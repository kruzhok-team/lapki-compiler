"""Module for managing platforms."""
from collections import defaultdict
from pprint import pprint
import json
import uuid
from typing import Dict, Set, List, DefaultDict

from aiofile import async_open
from aiopath import AsyncPath
from compiler.config import PLATFORM_DIRECTORY, LIBRARY_PATH
from compiler.types.inner_types import InnerFile

try:
    from .types.platform_types import Platform
except ImportError:
    from compiler.types.platform_types import Platform

PlatformId = str
PlatformVersion = str


async def _write_source(path: str, source_files: List[InnerFile]) -> None:
    for source in source_files:
        filename = f'{path}{source.filename}.{source.extension}'
        await AsyncPath(filename).parent.mkdir(parents=True, exist_ok=True)
        mode = 'wb' if isinstance(source.fileContent, bytes) else 'w'
        async with async_open(filename, mode) as f:
            await f.write(source.fileContent)


def _get_img_path(id: str, version: str) -> str:
    """Get path to images dir."""
    return _gen_platform_path(LIBRARY_PATH, id, version) + 'img/'


def _get_source_path(id: str, version: str) -> str:
    """Get path to source dir."""
    return _gen_platform_path(LIBRARY_PATH, id, version) + 'source/'


def _get_path_to_platform(id: str, version: str) -> str:
    """Get path to platform JSON scheme."""
    base_path = _gen_platform_path(PLATFORM_DIRECTORY, id, version)
    return base_path + f'{id}-{version}.json'


def _gen_platform_path(base_path: str, id: str, version: str) -> str:
    return (base_path + id +
            '/' + version + '/')


async def _read_platform_files(source_dir: str):
    async for source in AsyncPath(source_dir).rglob('*'):
        if not await source.is_file():
            continue
        async with async_open(source, 'r') as f:
            content = await f.read()
            extension = ''.join(source.suffixes).replace('.', '', 1)
            filename = str(source.relative_to(source_dir)).split('.')[0]
            yield InnerFile(filename=filename,
                            extension=extension,
                            fileContent=content)


class PlatformException(Exception):
    """Error during add platforms."""

    ...


class PlatformManager:
    """
    Класс-синглтон, отвечающий за загрузку платформ.

    TODO: А также их удаление из памяти, если их не используют
    какое-то время.
    """

    # Здесь будут храниться недавно использованные
    # платформы для быстрого доступа.
    platforms: Dict[str, Platform] = {}
    # Здесь будет храниться список id платформ.
    platforms_versions_info: DefaultDict[PlatformId, Set[PlatformVersion]] = (
        defaultdict(
            lambda: set())
    )

    @staticmethod
    def gen_platform_id() -> str:
        """Generate platform id."""
        return uuid.uuid4().hex

    @staticmethod
    async def save_platform(platform: Platform,
                            source_files: List[InnerFile],
                            images: List[InnerFile] | None = None) -> None:
        """
        Save platform to folder and add platform's\
            info to platforms_versions_info.

        Doesn't generate id.
        """
        platform_path = _gen_platform_path(
            PLATFORM_DIRECTORY, platform.id, platform.version)
        platform_library_path = _gen_platform_path(
            LIBRARY_PATH, platform.id, platform.version)
        source_path = _get_source_path(platform.id, platform.version)
        img_path = _get_img_path(platform.id, platform.version)
        json_platform = platform.model_dump_json(indent=4)
        await AsyncPath(platform_path).mkdir(parents=True, exist_ok=False)
        await AsyncPath(platform_library_path).mkdir(parents=True,
                                                     exist_ok=False)
        await AsyncPath(source_path).mkdir(exist_ok=False)
        await AsyncPath(img_path).mkdir(exist_ok=False)
        await _write_source(platform_path, [
            InnerFile(filename=f'{platform.id}-{platform.version}',
                      extension='json',
                      fileContent=json_platform)])
        await _write_source(source_path, source_files)

        if images is not None:
            await _write_source(img_path, images)
        PlatformManager.platforms_versions_info[platform.id].add(
            platform.version)

    @staticmethod
    async def load_platform(path_to_platform: str | AsyncPath) -> Platform:
        """Load platform from file and add it to dict."""
        try:
            async with async_open(path_to_platform, 'r') as f:
                unprocessed_platform_data: str = await f.read()
                platform = Platform(
                    **json.loads(unprocessed_platform_data))
                if PlatformManager.platforms.get(platform.id, None) is None:
                    PlatformManager.platforms[platform.id] = platform
                else:
                    print(f'Platform with id {platform.id} is already exists.')
                return platform
        except Exception as e:
            raise PlatformException(
                f'Во время обработки файла "{path_to_platform}"'
                f'произошла ошибка! {e}')

    @staticmethod
    async def init_platforms(path_to_schemas_dir: str) -> None:
        """Find platforms in directory and add it to Dict."""
        print(f'Поиск схем в папке "{path_to_schemas_dir}"...')
        async for path in AsyncPath(path_to_schemas_dir).glob('*json'):
            try:
                async with async_open(path, 'r') as f:
                    unprocessed_platform_data: str = await f.read()
                    platform = Platform(
                        **json.loads(unprocessed_platform_data))
                    PlatformManager.platforms_versions_info[platform.id].add(
                        platform.version)
            except Exception as e:
                print(
                    f'Во время обработки файла "{path.absolute()}"'
                    f'произошла ошибка! {e}')

        print('Были найдены платформы:')
        pprint(dict(PlatformManager.platforms_versions_info), indent=3)

    @staticmethod
    async def get_platform(platform_id: str, version: str) -> Platform:
        """Get platform object by id."""
        platform: Platform | None = PlatformManager.platforms.get(platform_id)

        if platform is not None:
            return platform

        if version not in PlatformManager.platforms_versions_info[platform_id]:
            raise PlatformException(
                f'Unsupported platform {platform_id}, version {version}')

        return await PlatformManager.load_platform(
            _get_path_to_platform(platform_id, version))

    @staticmethod
    async def get_raw_platform_scheme(platform_id: str, version: str) -> str:
        """Get raw platform JSON scheme."""
        platform: Platform | None = PlatformManager.platforms.get(platform_id)

        if platform is not None:
            return platform.model_dump_json(indent=4)

        if PlatformManager.platform_exist(platform_id, version):
            raise PlatformException(f'Unsupported platform {platform_id}')

        path_to_platform = _get_path_to_platform(platform_id, version)
        async with async_open(path_to_platform, 'r') as f:
            return await f.read()

    @staticmethod
    def platform_exist(platform_id: str, version: str) -> bool:
        """Check that platform exist."""
        return (version not in
                PlatformManager.platforms_versions_info[platform_id])

    @staticmethod
    async def get_platform_sources(platform_id: str, version: str):
        """Get platform source-code files by id and version."""
        source_dir = _get_source_path(platform_id, version)
        async for source in AsyncPath(source_dir).rglob('*'):
            if not await source.is_file():
                continue
            async with async_open(source, 'r') as f:
                content = await f.read()
                extension = ''.join(source.suffixes).replace('.', '', 1)
                filename = str(source.relative_to(source_dir)).split('.')[0]
                yield InnerFile(filename=filename,
                                extension=extension,
                                fileContent=content)

    @staticmethod
    async def get_platform_images(platform_id: str, version: str):
        """Get platform images by id and version."""
        source_dir = _get_img_path(platform_id, version)
        return _read_platform_files(source_dir)
