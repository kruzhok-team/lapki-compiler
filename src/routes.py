from aiohttp import web
from handler import Handler

def setup_routes(app : web.Application):
    app.add_routes([web.get("/ws", Handler.handle_ws_compile)])
    app.add_routes([web.get("/", Handler.handle_get_compile)])