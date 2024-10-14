"""Module implements sending errors."""

from aiohttp import web
from compiler.types.inner_types import (
    CompilerResponse,
    CommandResult,
    LegacyResponse,
    StateMachineResult
)


class RequestError:
    """Error during processing response."""

    def __init__(self, _error: str):
        self.error = _error

    async def dropConnection(
        self,
        ws: web.WebSocketResponse,
        legacy=False
    ) -> None:
        """Drop connection and send error."""
        if (not ws.closed):
            if (legacy):
                await ws.send_json(
                    LegacyResponse(
                        result=self.error,
                        return_code=-2,
                        stderr='',
                        stdout='',
                        binary=[],
                        source=[]
                    )
                )
            await ws.send_json(
                CompilerResponse(
                    result='NOTOK',
                    state_machines={
                        '': StateMachineResult(
                            result='NOTOK',
                            name='',
                            commands=[
                                CommandResult(
                                    command='compiler job',
                                    return_code=-2,
                                    stderr=self.error,
                                    stdout='')
                            ],
                            binary=[],
                            source=[])
                    }
                ).model_dump()
            )
            await ws.close()
