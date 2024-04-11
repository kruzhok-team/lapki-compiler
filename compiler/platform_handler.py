"""Module implements handling request to process platforms."""
from typing import Optional

from aiohttp import web
from compiler.config import MAX_MSG_SIZE
from compiler.types.platform_types import Platform


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
        platform = Platform(**await ws.receive_json())
        return ws
