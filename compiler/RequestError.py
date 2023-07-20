import aiohttp
import asyncjson
class RequestError:
    def __init__(self, _error):
        self.error = _error
    async def dropConnection(self, ws):
        await ws.send_json(await asyncjson.dumps(
                {
                    "result" : self.error
                }
            )
        )
        await ws.close()