from aiohttp import web
import asyncio

try:
    from .routes import setup_routes
    from .config import SERVER_PORT, SERVER_HOST, SCHEMA_DIRECTORY
    from .Logger import Logger
    from .PlatformManager import PlatformManager
except ImportError:
    from compiler.routes import setup_routes
    from compiler.config import SERVER_PORT
    from compiler.PlatformManager import PlatformManager


async def main():
    app = web.Application()
    setup_routes(app)
    runner = web.AppRunner(app)
    await runner.setup()
    
    site = web.TCPSite(runner, host=SERVER_HOST, port=SERVER_PORT)
    await Logger.init_logger()
    await PlatformManager.initPlatform(SCHEMA_DIRECTORY)
    await site.start()
    print("Модуль компилятора запущен...")
    while True:
        await asyncio.sleep(3600)


def sync_main():
    asyncio.run(main())


if __name__ == "__main__":
    sync_main()
