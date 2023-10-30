import json
from aiopath import AsyncPath
from aiofile import async_open

try:
    from Logger import Logger
except ImportError:
    from compiler.Logger import Logger


class PlatformManager:
    """
        Класс-синглтон, отвечающий за загрузку платформ,
        а также их удаление из памяти, если их неиспользуют
        какое-то время.
    """

    platforms: dict[str, dict] = {}

    # TODO: Валидация через pydantic
    @staticmethod
    async def initPlatform(path_to_schemas_dir: str) -> None:
        print(f"Поиск схем в папке '{path_to_schemas_dir}'...")
        async for path in AsyncPath(path_to_schemas_dir).glob("*json"):
            try:
                async with async_open(path, 'r') as f:
                    unprocessed_platform_data = await f.read()
                platform_data = json.loads(unprocessed_platform_data)
                platform_id = list(platform_data["platform"].keys())[0]
                platform = platform_data["platform"][platform_id]
                PlatformManager.platforms[platform_id] = platform
            except Exception as e:
                print(
                    f"Во время обработки файла '{path}' произошла ошибка! {e}")

        print(
            f"Были найдены платформы: {list(PlatformManager.platforms.keys())}")

    @staticmethod
    def getPlatform(platform: str) -> dict | None:
        return PlatformManager.platforms.get(platform)
