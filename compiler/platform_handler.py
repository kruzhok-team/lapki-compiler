"""Module implements handling request to process platforms."""
from typing import List, Optional

import aiohttp
from aiohttp import web
from compiler.PlatformManager import PlatformManager
from compiler.config import MAX_MSG_SIZE
from compiler.types.inner_types import File
from compiler.types.platform_types import Platform

# С помощью этих функций отделяется процесс передачи данных
# и бизнес-логика, также эти функции можно тестировать с помощью CI/CD,
# так как для них не требуется запуск всего компилятора в целом.


async def _add_platform(platform: Platform,
                        source_files: List[File],
                        images: List[File]) -> str:
    platform.id = PlatformManager.gen_platform_id()
    await PlatformManager.save_platform(platform, source_files, images)
    return platform.id


async def _get_platform(platform_id: str, version: str) -> str:
    return await PlatformManager.get_raw_platform_scheme(
        platform_id, version)


class PlatformHandler:
    """Class for handling requests CRUD-operations with platforms."""

    @staticmethod
    async def handle_add_platform(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None
    ) -> web.WebSocketResponse:
        """Validate and save platform."""
        if ws is None:
            ws = web.WebSocketResponse(
                autoclose=False, max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)

        # TODO: Отлавливание ошибок и отправка их пользователю
        platform = Platform(**await ws.receive_json())
        source_files: List[File] = []
        images: List[File] = []
        if platform.compile:
            # Если платформа компилируемая, ждем исходники
            async for msg in ws:
                if msg.type != aiohttp.WSMsgType.TEXT:
                    continue
                match msg.data:
                    case 'stop':
                        break
                    case 'file':
                        file = File(**await ws.receive_json())
                        source_files.append(file)
                    case _:
                        await ws.close()
                        return ws

        if platform.visual:
            async for msg in ws:
                if msg.type != aiohttp.WSMsgType.TEXT:
                    continue
                match msg.data:
                    case 'stop':
                        break
                    case 'img':
                        img = File(**await ws.receive_json())
                        images.append(img)
                    case _:
                        await ws.close()
                        return ws

        platform_id = await _add_platform(platform, source_files, images)
        await ws.send_str('id')
        await ws.send_str(platform_id)
        return ws

    @staticmethod
    async def handle_get_platform_by_id(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None
    ) -> web.WebSocketResponse:
        """Get platform json scheme."""
        if ws is None:
            ws = web.WebSocketResponse(
                autoclose=False, max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)

        platform_id = await ws.receive_str()
        version = await ws.receive_str()

        raw_platform = await _get_platform(platform_id, version)
        await ws.send_str('raw-platform-scheme')
        await ws.send_str(raw_platform)
        return ws
