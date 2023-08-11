import aiohttp
import json
from aiofile import async_open
from pathlib import Path


class Client:
    def __init__(self):
        pass
    
    async def doConnect(self, addr: str):
        session = aiohttp.ClientSession()
        self.ws = await session.ws_connect(addr)
    
    async def sendSMJson(self, path: str) -> dict:
        async with async_open(path, 'r') as req:
            json_data: str = await req.read()

        await self.ws.send_str(json_data)
        response = await self.ws.receive_json()

        return response
        
    async def importBerlogaScheme(self, path):
        async with async_open(path, 'r') as req:
            data = await req.read()
        
        await self.ws.send_str(json.dumps(data))
        response = json.dumps(await self.ws.receive_json(), indent=4, ensure_ascii=False)

        return response

    async def exportBerlogaScheme(self, path):
        async with async_open(path, 'r') as req:
            data = json.loads(await req.read())
        
        await self.ws.send_json(json.dumps(data))
        response = await self.ws.receive_str()

        return response

    async def sendMultiFileProject(self, dir_path: str, compiler: str, flags: list, globs):
        path = Path(dir_path)
        
        if path.is_dir():
            request = {
                "source": [],
                "compilerSettings": {}
            }
            i = 0
            for g in globs:
                print(g)
                for file in path.glob(g):
                    if file.is_file():
                        request["source"].append({})
                        request["source"][i]["filename"] = file.stem
                        request["source"][i]["extension"] = file.suffix
                        async with async_open(file, 'r') as f:
                            request["source"][i]["fileContent"] = await f.read()
                        i += 1
            request["compilerSettings"]["compiler"] = compiler
            request["compilerSettings"]["flags"] = flags
                
            await self.ws.send_json(json.dumps(request))
            
            return request
        
        raise Exception(f"{dir_path} is not a directory")
