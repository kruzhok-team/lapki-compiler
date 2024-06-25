"""Root module."""
from typing import NoReturn
import asyncio

from aiohttp import web
from compiler.routes import setup_routes
from compiler.config import (
    configure,
    get_config
)
from compiler.PlatformManager import PlatformManager
from compiler.access_controller import AccessController
from compiler.Logger import Logger
from compiler.types.config_types import ArgumentParser


async def main() -> NoReturn:
    """Config and running app."""
    args_parser = ArgumentParser()
    app = web.Application()
    setup_routes(app)
    configure(args_parser)
    config = get_config()
    runner = web.AppRunner(app)
    await runner.setup()
    platform_manager = PlatformManager()
    access_controller = AccessController()
    await access_controller.init_access_tokens()
    site = web.TCPSite(runner, host=config.server_host,
                       port=config.server_port)
    await Logger.init_logger()
    await platform_manager.init_platforms(config.platform_directory)
    await site.start()
    print('Модуль компилятора запущен...')
    while True:
        await asyncio.sleep(3600)


def sync_main():
    """Run compiler."""
    asyncio.run(main())


if __name__ == '__main__':
    sync_main()
