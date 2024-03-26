"""Module for managing platforms."""
import json

from aiofile import async_open
from aiopath import AsyncPath

try:
    from .types.platform_types import Platform
except ImportError:
    from compiler.types.platform_types import Platform


class PlatformException(Exception):
    """Error during add platforms."""

    ...


class PlatformManager:
    """
    Класс-синглтон, отвечающий за загрузку платформ.

    А также их удаление из памяти, если их не используют
    какое-то время.
    """

    platforms: dict[str, Platform] = {}

    @staticmethod
    async def initPlatform(path_to_schemas_dir: str) -> None:
        """Find platforms in path and add it to Dict."""
        print(f'Поиск схем в папке "{path_to_schemas_dir}"...')
        async for path in AsyncPath(path_to_schemas_dir).glob('*json'):
            try:
                async with async_open(file_specifier=path, mode='r') as f:
                    unprocessed_platform_data = await f.read()
                platform = Platform(
                    **json.loads(unprocessed_platform_data))
                if PlatformManager.platforms.get(platform.id, None) is None:
                    PlatformManager.platforms[platform.id] = platform
                else:
                    print(f'Platform with id {platform.id} is already exists.')
            except Exception as e:
                print(
                    f'Во время обработки файла "{path}" произошла ошибка! {e}')

        print(
            f'Были найдены платформы: {list(PlatformManager.platforms.keys())}'
        )

    @staticmethod
    def getPlatform(platform_id: str) -> Platform:
        """Get platform by id."""
        platform: Platform | None = PlatformManager.platforms.get(platform_id)
        if platform is None:
            raise PlatformException(f'Unsupported platform {platform_id}')
        return platform
