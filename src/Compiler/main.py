from aiohttp import web
import asyncio

from routes import setup_routes

async def main():
    app = web.Application()
    setup_routes(app)
    runner = web.AppRunner(app)
    await runner.setup() 
    site = web.TCPSite(runner, host="localhost", port=8080)
    await site.start()

    while True:
        await asyncio.sleep(3600)
    
if __name__ == "__main__":
    asyncio.run(main())