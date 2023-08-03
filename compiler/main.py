from aiohttp import web
import asyncio

try:
    from .routes import setup_routes
    from .config import SERVER_PORT
    from .Logger import Logger
except ImportError:
    from compiler.routes import setup_routes
    from compiler.config import SERVER_PORT


async def main():
    Logger.init_logger()
    app = web.Application()
    setup_routes(app)
    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, host="localhost", port=SERVER_PORT)
    await site.start()
    print("Модуль компилятора запущен...")
    while True:
        await asyncio.sleep(3600)


def sync_main():
    asyncio.run(main())


if __name__ == "__main__":
    sync_main()
