"""Module implements handling request to process platforms."""
from typing import List, Optional, Set, Dict

import aiohttp
from aiohttp import web
from compiler.Logger import Logger
from compiler.PlatformManager import (
    PlatformException,
    PlatformManager,
    PlatformId,
    PlatformMeta
)
from compiler.RequestError import RequestError
from compiler.access_controller import (
    AccessController,
    AccessControllerException
)
from compiler.config import MAX_MSG_SIZE
from compiler.types.inner_types import File
from compiler.types.platform_types import Platform


class PlatformHandlerException(Exception):
    """Errors during process platform's request."""

    ...

# С помощью этих функций отделяется процесс передачи данных
# и бизнес-логика, также эти функции можно тестировать с помощью CI/CD,
# так как для них не требуется запуск всего компилятора в целом.


def _get_platforms_list() -> Dict[PlatformId, PlatformMeta]:
    platform_manager = PlatformManager()
    return platform_manager.versions_info


def _check_token(token: str) -> None:
    access_controller = AccessController()
    if not access_controller.check_access_token(token):
        raise AccessControllerException('Invalid token.')


async def _delete_platform_by_versions(platform_id: str,
                                       versions_to_delete: str) -> None:
    platform_manager = PlatformManager()
    set_versions: Set[str] = set(map(
        lambda string: string.strip(), versions_to_delete.split(',')))
    new_versions_info = await platform_manager.delete_platform_by_versions(
        platform_id,
        set_versions
    )
    platform_manager.set_platforms_info(new_versions_info)


async def _add_platform(platform: Platform,
                        source_files: List[File],
                        images: List[File]) -> str:
    platform_manager = PlatformManager()
    platform.id = platform_manager.gen_platform_id()
    new_versions_info = await platform_manager.add_platform(
        platform,
        source_files,
        images)
    platform_manager.set_platforms_info(new_versions_info)
    return platform.id


async def _update_platform(new_platform: Platform,
                           source_files: List[File],
                           images: List[File]
                           ) -> None:
    platform_manager = PlatformManager()
    new_versions_info = await platform_manager.update_platform(
        new_platform,
        source_files,
        images)
    platform_manager.set_platforms_info(new_versions_info)


async def _get_platform(platform_id: str, version: str) -> str:
    platform_manager = PlatformManager()
    return await platform_manager.get_raw_platform_scheme(
        platform_id, version)


async def _delete_platform(platform_id: str) -> None:
    platform_manager = PlatformManager()
    new_versions_info = await platform_manager.delete_platform(platform_id)
    platform_manager.set_platforms_info(new_versions_info)

Images = List[File]
SourceFiles = List[File]


async def _get_platform_sources(ws: web.WebSocketResponse,
                                visual: bool,
                                compile: bool) -> tuple[Images, SourceFiles]:
    source_files: List[File] = []
    images: List[File] = []
    if compile:
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
                    img = File(**await ws.receive_json())
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
        ws: Optional[web.WebSocketResponse] = None,
        access_token: Optional[str] = None
    ) -> web.WebSocketResponse:
        """Validate and save platform."""
        ws = await _prepare_request(ws, request)
        try:
            if access_token is None:
                access_token = await ws.receive_str()
            _check_token(access_token)
            # TODO: Отлавливание ошибок и отправка их пользователю
            platform = Platform(**await ws.receive_json())
            images, source_files = await _get_platform_sources(
                ws, platform.visual, platform.compile)
            platform_id = await _add_platform(platform, source_files, images)
            await ws.send_str('id')
            await ws.send_str(platform_id)
        except AccessControllerException as e:
            await RequestError(str(e)).dropConnection(ws)
        except PlatformException as e:
            await RequestError(str(e)).dropConnection(ws)
        except Exception:
            await Logger.logException()
            await RequestError('Internal error.').dropConnection(ws)
        return ws

    @staticmethod
    async def handle_get_platform_by_id(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None
    ) -> web.WebSocketResponse:
        """Get platform json scheme."""
        ws = await _prepare_request(ws, request)
        try:
            platform_id = await ws.receive_str()
            version = await ws.receive_str()
            raw_platform = await _get_platform(platform_id, version)
            await ws.send_str('raw-platform-scheme')
            await ws.send_str(raw_platform)
        except PlatformException as e:
            await RequestError(str(e)).dropConnection(ws)
        except Exception:
            await Logger.logException()
            await RequestError('Internal error.').dropConnection(ws)
        return ws

    @staticmethod
    async def handle_get_platform_source_files(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None
    ) -> web.WebSocketResponse:
        """Get platform source-code files."""
        ws = await _prepare_request(ws, request)
        try:
            platform_manager = PlatformManager()
            platform_id = await ws.receive_str()
            version = await ws.receive_str()
            source_generator = await platform_manager.get_platform_sources(
                platform_id, version)
            async for source in source_generator:
                await ws.send_str('source')
                await ws.send_json(source.model_dump())
            await ws.send_str('end-sources-send')
        except PlatformException as e:
            await RequestError(str(e)).dropConnection(ws)
        except Exception:
            await Logger.logException()
            await RequestError('Internal error.').dropConnection(ws)
        return ws

    @staticmethod
    async def handle_get_platform_images(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None,
    ) -> web.WebSocketResponse:
        """Get platform s images."""
        ws = await _prepare_request(ws, request)
        try:
            platform_id = await ws.receive_str()
            version = await ws.receive_str()
            platform_manager = PlatformManager()
            image_generator = await platform_manager.get_platform_images(
                platform_id, version)
            async for image in image_generator:
                await ws.send_str('img')
                await ws.send_json(image.model_dump())
            await ws.send_str('end-images-send')
        except PlatformException as e:
            await RequestError(str(e)).dropConnection(ws)
        except Exception:
            await Logger.logException()
            await RequestError('Internal error.').dropConnection(ws)
        return ws

    @staticmethod
    async def handle_update_platform(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None,
        access_token: Optional[str] = None
    ) -> web.WebSocketResponse:
        """Update platform by id."""
        ws = await _prepare_request(ws, request)
        try:
            if access_token is None:
                access_token = await ws.receive_str()
            _check_token(access_token)
            platform = Platform(**await ws.receive_json())
            images, source_files = await _get_platform_sources(
                ws, platform.visual, platform.compile)
            await _update_platform(platform,
                                   source_files,
                                   images)
            await ws.send_str('updated')
        except AccessControllerException as e:
            await RequestError(str(e)).dropConnection(ws)
        except PlatformException as e:
            await RequestError(str(e)).dropConnection(ws)
        except Exception:
            await Logger.logException()
            await RequestError('Internal error.').dropConnection(ws)
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
        try:
            if access_token is None:
                access_token = await ws.receive_str()
            _check_token(access_token)
            platform_id = await ws.receive_str()
            versions_to_delete = await ws.receive_str()
            await _delete_platform_by_versions(platform_id, versions_to_delete)
            await ws.send_str('deleted')
        except AccessControllerException as e:
            await RequestError(str(e)).dropConnection(ws)
        except PlatformException as e:
            await RequestError(str(e)).dropConnection(ws)
        except Exception:
            await Logger.logException()
            await RequestError('Internal error.').dropConnection(ws)
        return ws

    @staticmethod
    async def handle_remove_platform(
        request: web.Request,
        ws: Optional[web.WebSocketResponse] = None,
        access_token: str | None = None
    ) -> web.WebSocketResponse:
        """Remove all versions of platform."""
        ws = await _prepare_request(ws, request)
        try:
            if access_token is None:
                access_token = await ws.receive_str()
            _check_token(access_token)
            platform_id = await ws.receive_str()
            await _delete_platform(platform_id)
            await ws.send_str('deleted')
        except AccessControllerException as e:
            await RequestError(str(e)).dropConnection(ws)
        except PlatformException as e:
            await RequestError(str(e)).dropConnection(ws)
        except Exception:
            await Logger.logException()
            await RequestError('Internal error.').dropConnection(ws)
        return ws

    @staticmethod
    async def handle_auth(ws: web.WebSocketResponse) -> str | None:
        """Check token."""
        try:
            token = await ws.receive_str()
            _check_token(token)
            await ws.send_str('auth_success')
            return token
        except AccessControllerException as e:
            await RequestError(str(e)).dropConnection(ws)
        except Exception:
            await Logger.logException()
            await RequestError('Internal error.').dropConnection(ws)
        return None

    @staticmethod
    async def handle_get_list(request: web.Request,
                              ws: Optional[web.WebSocketResponse] = None
                              ) -> web.WebSocketResponse:
        """Get list of all platforms."""
        ws = await _prepare_request(ws, request)
        try:
            platform_list = _get_platforms_list()
            await ws.send_json(platform_list)
            return ws
        except Exception:
            await Logger.logException()
            await RequestError('Internal error.').dropConnection(ws)
        return ws
