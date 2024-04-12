"""Module implements handling request to process platforms."""
from typing import List, Optional, Set

import aiohttp
from aiohttp import web
from compiler.PlatformManager import PlatformManager
from compiler.config import MAX_MSG_SIZE
from compiler.types.inner_types import InnerFile
from compiler.types.platform_types import Platform


class PlatformHandlerException(Exception):
    """Errors during process platform's request."""

    ...

# С помощью этих функций отделяется процесс передачи данных
# и бизнес-логика, также эти функции можно тестировать с помощью CI/CD,
# так как для них не требуется запуск всего компилятора в целом.


async def _delete_platform_by_versions(platform_id: str,
                                       versions_to_delete: str) -> None:
    set_versions: Set[str] = set(map(
        lambda string: string.strip(), versions_to_delete.split(',')))
    await PlatformManager.delete_platform_by_versions(platform_id,
                                                      set_versions)


async def _add_platform(platform: Platform,
                        source_files: List[InnerFile],
                        images: List[InnerFile]) -> str:
    platform.id = PlatformManager.gen_platform_id()
    await PlatformManager.add_platform(platform, source_files, images)
    return platform.id


async def _update_platform(new_platform: Platform,
                           access_token: str,
                           source_files: List[InnerFile],
                           images: List[InnerFile]
                           ) -> None:
    return await PlatformManager.update_platform(new_platform,
                                                 source_files,
                                                 images)


async def _get_platform(platform_id: str, version: str) -> str:
    return await PlatformManager.get_raw_platform_scheme(
        platform_id, version)


Images = List[InnerFile]
SourceFiles = List[InnerFile]


async def _get_platform_sources(ws: web.WebSocketResponse,
                                visual: bool,
                                compile: bool) -> tuple[Images, SourceFiles]:
    source_files: List[InnerFile] = []
    images: List[InnerFile] = []
    if compile:
        # Если платформа компилируемая, ждем исходники
        async for msg in ws:
            if msg.type != aiohttp.WSMsgType.TEXT:
                continue
            match msg.data:
                case 'stop':
                    break
                case 'file':
                    file = InnerFile(**await ws.receive_json())
                    source_files.append(file)
                case _:
                    raise PlatformHandlerException(
                        f'Unknown command {msg.data}!')
    if visual:
        async for msg in ws:
            if msg.type != aiohttp.WSMsgType.TEXT:
                continue
            match msg.data:
                case 'stop':
                    break
                case 'img':
                    img = InnerFile(**await ws.receive_json())
                    images.append(img)
                case _:
                    raise PlatformHandlerException(
                        f'Unknown command {msg.data}')
    return images, source_files


async def _prepare_request(ws: Optional[web.WebSocketResponse],
                           request: web.Request) -> web.WebSocketResponse:
    if ws is None:
        ws = web.WebSocketResponse(
            autoclose=False, max_msg_size=MAX_MSG_SIZE)
        await ws.prepare(request)
    return ws


class PlatformHandler:
    """Class for handling requests CRUD-operations with platforms."""

    @staticmethod
    async def handle_add_platform(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None
    ) -> web.WebSocketResponse:
        """Validate and save platform."""
        ws = await _prepare_request(ws, request)

        # TODO: Отлавливание ошибок и отправка их пользователю
        platform = Platform(**await ws.receive_json())
        images, source_files = await _get_platform_sources(
            ws, platform.visual, platform.compile)

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
        ws = await _prepare_request(ws, request)

        platform_id = await ws.receive_str()
        version = await ws.receive_str()

        raw_platform = await _get_platform(platform_id, version)
        await ws.send_str('raw-platform-scheme')
        await ws.send_str(raw_platform)
        return ws

    @staticmethod
    async def handle_get_platform_source_files(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None
    ) -> web.WebSocketResponse:
        """Get platform source-code files."""
        # TODO: Проверить, что платформа компилируемая
        ws = await _prepare_request(ws, request)

        platform_id = await ws.receive_str()
        version = await ws.receive_str()
        source_generator = await PlatformManager.get_platform_sources(
            platform_id, version)
        async for source in source_generator:
            await ws.send_str('source')
            await ws.send_json(source.model_dump())
        await ws.send_str('end-sources-send')
        return ws

    @staticmethod
    async def handle_get_platform_images(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None,
        access_token: str | None = None
    ) -> web.WebSocketResponse:
        """Get platform s images."""
        # TODO: Проверить, что платформа визуальная
        if ws is None:
            ws = web.WebSocketResponse(
                autoclose=False, max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)
        if access_token is None:
            ...
        platform_id = await ws.receive_str()
        version = await ws.receive_str()
        image_generator = await PlatformManager.get_platform_images(
            platform_id, version)
        async for image in image_generator:
            await ws.send_str('img')
            await ws.send_json(image.model_dump())
        await ws.send_str('end-images-send')
        return ws

    @staticmethod
    async def handle_update_platform(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None,
        access_token: str | None = None
    ) -> web.WebSocketResponse:
        """Update platform by id."""
        if ws is None:
            ws = web.WebSocketResponse(
                autoclose=False, max_msg_size=MAX_MSG_SIZE)
            await ws.prepare(request)
        if access_token is None:
            await ws.send_str('send-token')
            access_token = await ws.receive_str()
            # TODO проверка токена
        platform = Platform(**await ws.receive_json())
        images, source_files = await _get_platform_sources(
            ws, platform.visual, platform.compile)
        await _update_platform(platform,
                               access_token,
                               source_files,
                               images)
        await ws.send_str('updated')
        return ws

    @staticmethod
    async def handle_remove_platform_by_versions(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None,
        access_token: str | None = None
    ) -> web.WebSocketResponse:
        """Remove platform by versions and platform id.

        Versions is a string like "v1.0, 2.0, 3.0".
        """
        ws = await _prepare_request(ws, request)
        platform_id = await ws.receive_str()
        versions_to_delete = await ws.receive_str()
        await _delete_platform_by_versions(platform_id, versions_to_delete)

        return ws
