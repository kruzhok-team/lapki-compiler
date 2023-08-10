from aiohttp import web


class RequestError:

    def __init__(self, _error):
        self.error = _error

    async def dropConnection(self, ws: web.WebSocketResponse):
        if (not ws.closed):
            await ws.send_json(
                    {
                        "result": self.error,
                        "return code": '',
                        "stdout": '',
                        "stderr": '',
                        "binary": [],
                        "source": []
                    }
            )
            await ws.close()
