import json
from aiopath import AsyncPath
from aiofile import async_open

from compiler.types.platform_types import UnprocessedPlatform

try:
    from .Logger import Logger
    from .types.platform_types import Platform
except ImportError:
    from compiler.types.platform_types import Platform


class PlatformException(Exception):
    ...


class PlatformManager:
    """
        Класс-синглтон, отвечающий за загрузку платформ,
        а также их удаление из памяти, если их неиспользуют
        какое-то время.
    """

    platforms: dict[str, Platform] = {}

    @staticmethod
    async def initPlatform(path_to_schemas_dir: str) -> None:
        print(f"Поиск схем в папке '{path_to_schemas_dir}'...")
        async for path in AsyncPath(path_to_schemas_dir).glob("*json"):
            try:
                async with async_open(path, 'r') as f:
                    unprocessed_platform_data = await f.read()
                platform_data = UnprocessedPlatform(**json.loads(unprocessed_platform_data))
                platform_id = list(platform_data.platform.keys())[0]
                platform = platform_data.platform[platform_id]
                PlatformManager.platforms[platform_id] = platform
            except Exception as e:
                print(
                    f"Во время обработки файла '{path}' произошла ошибка! {e}")

        print(
            f"Были найдены платформы: {list(PlatformManager.platforms.keys())}")

    @staticmethod
    def getPlatform(platform_id: str) -> Platform:
        platform = PlatformManager.platforms.get(platform_id)
        if platform is None:
            raise PlatformException(f'Unsupported platform {platform_id}')
        return platform
