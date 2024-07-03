"""Module for working with routes."""
from aiohttp import web
from compiler.main_handler import main_handle
from compiler.handler import Handler
from compiler.platform_handler import PlatformHandler
from compiler.raw_compilation import handle_ws_raw_compile


def setup_routes(app: web.Application) -> None:
    """Set up routes handlers."""
    app.add_routes(
        [
            web.get('/main', main_handle),
            web.get('/ws', Handler.handle_ws_compile),
            web.get('/ws/raw_compilation', handle_ws_raw_compile),
            web.get('/ws/berloga/import', Handler.handle_berloga_import),
            web.get('/ws/berloga/export', Handler.handle_berloga_export),
            web.get('/cgml', Handler.handle_cgml_compile),
            web.get('/platform/add', PlatformHandler.handle_add_platform),
            web.get('/platform/delete',
                    PlatformHandler.handle_remove_platform),
            web.get('/platform/delete_by_versions',
                    PlatformHandler.handle_remove_platform_by_versions),
            web.get('/platform/update',
                    PlatformHandler.handle_update_platform),
            web.get('/platform/get_json',
                    PlatformHandler.handle_get_platform_by_id),
            web.get('/platform/get_images',
                    PlatformHandler.handle_get_platform_images),
            web.get('/platform/get_sources',
                    PlatformHandler.handle_get_platform_source_files)
        ]
    )
