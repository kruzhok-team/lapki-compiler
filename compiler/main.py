"""Root module."""
from typing import NoReturn
import asyncio

from aiohttp import web

try:
    from .routes import setup_routes
    from .config import SERVER_PORT, SERVER_HOST, PLATFORM_DIRECTORY
    from .Logger import Logger
    from .access_controller import AccessController
    from .PlatformManager import PlatformManager
except ImportError:
    from compiler.routes import setup_routes
    from compiler.config import SERVER_PORT, SERVER_HOST, PLATFORM_DIRECTORY
    from compiler.PlatformManager import PlatformManager
    from compiler.access_controller import AccessController
    from .Logger import Logger


async def main() -> NoReturn:
    """Config and running app."""
    app = web.Application()
    setup_routes(app)
    runner = web.AppRunner(app)
    await runner.setup()

    platform_manager = PlatformManager()
    access_controller = AccessController()
    await access_controller.init_access_tokens()
    site = web.TCPSite(runner, host=SERVER_HOST, port=SERVER_PORT)
    await Logger.init_logger()
    await platform_manager.init_platforms(PLATFORM_DIRECTORY)
    await site.start()
    print('Модуль компилятора запущен...')
    while True:
        await asyncio.sleep(3600)


def sync_main():
    """Run compiler."""
    asyncio.run(main())


if __name__ == '__main__':
    sync_main()
