"""Module implements sending errors."""

from aiohttp import web
from compiler.types.inner_types import StateMachineResult


class RequestError:
    """Error during processing response."""

    def __init__(self, _error: str):
        self.error = _error

    async def dropConnection(self, ws: web.WebSocketResponse) -> None:
        """Drop connection and send error."""
        if (not ws.closed):
            await ws.send_json(
                StateMachineResult(
                    result=self.error,
                    return_code=-2,
                    stdout='',
                    stderr='',
                    binary=[],
                    source=[]
                ).model_dump()
            )
            await ws.close()
