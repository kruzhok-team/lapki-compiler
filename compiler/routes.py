"""Module for working with routes."""
from aiohttp import web

try:
    from .handler import Handler
except ImportError:
    from compiler.handler import Handler


def setup_routes(app: web.Application) -> None:
    """Set up routes handlers."""
    app.add_routes([web.get('/main', Handler.main)])
    app.add_routes([web.get('/ws', Handler.handle_ws_compile)])
    app.add_routes([web.get('/ws/source', Handler.handle_ws_compile_source)])
    app.add_routes(
        [web.get('/ws/berloga/import', Handler.handle_berloga_import)])
    app.add_routes(
        [web.get('/ws/berloga/export', Handler.handle_berloga_export)])
