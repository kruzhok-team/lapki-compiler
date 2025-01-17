"""Module implements sending errors."""

from typing import Dict

from aiohttp import web
from compiler.types.inner_types import (
    CompilerResponse,
    CommandResult,
    LegacyResponse,
    StateMachineResult
)


async def send_error(
    ws: web.WebSocketResponse,
    error: str,
):
    """Drop connection and send error without sm pinning."""
    if not ws.closed:
        await ws.send_str(error)
        await ws.close()


async def send_sm_error(
    ws: web.WebSocketResponse,
    state_machines: Dict[str, str],
    legacy=False,
) -> None:
    """Drop connection and send error with sm pinning."""
    if (not ws.closed):
        if (legacy):
            await ws.send_json(
                LegacyResponse(
                    result=list(state_machines.values())[0],
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
                    sm_id: StateMachineResult(
                        result='NOTOK',
                        name='',
                        commands=[
                            CommandResult(
                                command='compiler job',
                                return_code=-2,
                                stderr=error,
                                stdout='')
                        ],
                        binary=[],
                        source=[])
                    for sm_id, error in state_machines.items()
                }
            ).model_dump()
        )
        await ws.close()
