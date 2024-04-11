"""Module for managing platforms."""
import json
import uuid
from typing import Dict, Set, List

from aiofile import async_open
from aiopath import AsyncPath
from compiler.config import PLATFORM_DIRECTORY, LIBRARY_PATH
from compiler.types.inner_types import File

try:
    from .types.platform_types import Platform
except ImportError:
    from compiler.types.platform_types import Platform

PlatformId = str
PlatformVersion = str


async def _write_source(path: str, source_files: List[File]) -> None:
    for source in source_files:
        filename = f'{path}{source.filename}.{source.extension}'
        await AsyncPath(filename).parent.mkdir(parents=True, exist_ok=True)
        mode = 'wb' if isinstance(source.fileContent, bytes) else 'w'
        async with async_open(filename, mode) as f:
            await f.write(source.fileContent)


def _gen_platform_path(base_path: str, id: str, version: str) -> str:
    return (base_path + id +
            '/' + version + '/')


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
    platforms: dict[str, Platform] = {}
    # Здесь будет храниться список id платформ.
    platforms_versions_info: Dict[PlatformId, Set[PlatformVersion]] = {}

    @staticmethod
    def gen_platform_id() -> str:
        """Generate platform id."""
        return uuid.uuid4().hex

    @staticmethod
    async def save_platform(platform: Platform,
                            source_files: List[File],
                            images: List[File] | None = None) -> None:
        """
        Save platform to folder.

        Doesn't generate id.
        """
        platform_path = _gen_platform_path(
            PLATFORM_DIRECTORY, platform.id, platform.version)
        platform_library_path = _gen_platform_path(
            LIBRARY_PATH, platform.id, platform.version)
        source_path = platform_library_path + '/source/'
        img_path = platform_library_path + '/img/'
        json_platform = platform.model_dump_json(indent=4)
        await AsyncPath(platform_path).mkdir(parents=True)
        await AsyncPath(platform_library_path).mkdir(parents=True)
        await AsyncPath(source_path).mkdir()
        await AsyncPath(img_path).mkdir()
        await _write_source(platform_path, [File(
            f'{platform.id}-{platform.version}', 'json', json_platform)])
        await _write_source(source_path, source_files)

        if images is not None:
            await _write_source(img_path, images)

    @staticmethod
    async def load_platform(path_to_platform: str | AsyncPath) -> None:
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
        except Exception as e:
            print(
                f'Во время обработки файла "{path_to_platform}"'
                f'произошла ошибка! {e}')

    @staticmethod
    async def init_platforms(path_to_schemas_dir: str) -> None:
        """Find platforms in path and add it to Dict."""
        print(f'Поиск схем в папке "{path_to_schemas_dir}"...')
        async for path in AsyncPath(path_to_schemas_dir).glob('*json'):
            await PlatformManager.load_platform(path)

        print(
            f'Были найдены платформы: {list(PlatformManager.platforms.keys())}'
        )

    @staticmethod
    def get_platform(platform_id: str, version: str) -> Platform:
        """Get platform by id."""
        platform: Platform | None = PlatformManager.platforms.get(platform_id)
        if platform is None:
            raise PlatformException(f'Unsupported platform {platform_id}')
        return platform
