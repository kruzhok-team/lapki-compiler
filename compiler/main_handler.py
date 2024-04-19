"""Module implement main route handler."""
import aiohttp
from aiohttp import web
from compiler.Logger import Logger
from compiler.config import MAX_MSG_SIZE
from compiler.handler import Handler
from compiler.platform_handler import PlatformHandler


async def main_handle(request: web.Request) -> web.WebSocketResponse:
    """Root handler, call other handlers."""
    ws = web.WebSocketResponse(autoclose=False, max_msg_size=MAX_MSG_SIZE)
    await ws.prepare(request)
    await Logger.logger.info(request)
    token: str | None = None
    async for msg in ws:
        await Logger.logger.info(msg)
        if msg.type == aiohttp.WSMsgType.TEXT:
            match msg.data:
                case 'close':
                    await ws.close()
                case 'arduino':
                    # Я не понимаю, у pyright какие-то проблемы
                    # с моими асинхронными функциями
                    # type: ignore
                    await Handler.handle_ws_compile(request, ws)
                case 'berlogaImport':
                    # type: ignore
                    await Handler.handle_berloga_import(request, ws)
                case 'berlogaExport':
                    await Handler.handle_berloga_export(request, ws)
                case 'cgml':
                    await Handler.handle_cgml_compile(request, ws)
                case 'add_platform':
                    await PlatformHandler.handle_add_platform(
                        request,
                        ws,
                        token
                    )
                case 'remove_platform':
                    await PlatformHandler.handle_remove_platform(
                        request,
                        ws,
                        token
                    )
                case 'remove_platform_versions':
                    await PlatformHandler.handle_remove_platform_by_versions(
                        request,
                        ws,
                        token
                    )
                case 'update_platform':
                    await PlatformHandler.handle_update_platform(
                        request,
                        ws,
                        token
                    )
                case 'get_platform_json':
                    await PlatformHandler.handle_get_platform_by_id(
                        request,
                        ws
                    )
                case 'get_platform_images':
                    await PlatformHandler.handle_get_platform_images(
                        request,
                        ws
                    )
                case 'get_platform_sources':
                    await PlatformHandler.handle_get_platform_source_files(
                        request,
                        ws
                    )
                case 'auth':
                    token = await PlatformHandler.handle_auth(ws)
                case _:
                    await ws.send_str(f'Unknown {msg}!'
                                      'Use close, arduino,'
                                      'berlogaImport, berlogaExport')
        elif msg.type == aiohttp.WSMsgType.ERROR:
            pass

    return ws
