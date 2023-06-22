import aiohttp
from aiofile import async_open
import json
import base64

class Client:
    def __init__(self):
        pass
    
    async def doConnect(self, addr : str):
        session = aiohttp.ClientSession()
        self.ws = await session.ws_connect(addr)
    
    async def sendSMJson(self, path: str):
        async with async_open(path, 'r') as req:
            json_data = json.loads(await req.read())
        
        await self.ws.send_json(json_data)
        
    async def getResult(self):
        response = await self.ws.receive_json()
        json_response = json.loads(response)
        result = json_response["result"]
        
        if result == 'OK':
            for k in list(json_response.keys())[:-1]:
                print(f"{k}: {json_response[k]}")
            binary = json_response["binary"]
            binary = binary.encode('ascii')
            # binary = base64.b64decode(binary)

        return json_response
    
    async def sendSourceFile(self, path, compiler, flags):
        async with async_open(path, 'r') as req:
            data = await req.read()
        
        await self.ws.send_json(
            {
                # "dataFormat" : "source",
                
            }
        )
        