class PlatformManager:
    """
        Класс-синглтон, отвечающий за загрузку платформ,
        а также их удаление из памяти, если их неиспользуют
        какое-то время.
    """

    platforms: dict[str, dict] = {}

    @staticmethod
    def initPlatform(platform_data: dict, name: str):
        PlatformManager._parsePlatform(platform_data)
        
    @staticmethod
    def _parsePlatform(platform_data: dict) -> bool:
        return False