import aiohttp
from aiofile import async_open
import json
import base64
from pathlib import Path
class Client:
    def __init__(self):
        pass
    
    async def doConnect(self, addr: str):
        session = aiohttp.ClientSession()
        self.ws = await session.ws_connect(addr)
    
    async def sendSMJson(self, path: str):
        async with async_open(path, 'r') as req:
            json_data = json.loads(await req.read())
        
        await self.ws.send_json(json.dumps(json_data))
    
    #deprecated
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
    #deprecated
    async def sendSourceFile(self, path, compiler, flags):
        async with async_open(path, 'r') as req:
            data = await req.read()
        
        await self.ws.send_json(json.dumps(
            {
                "source": data,
                "compilerSettings": {
                    "filename": Path(path).name,
                    "compiler": compiler,
                    "flags": flags
                } 
            }
        )
        )
    
    async def sendMultiFileProject(self, dir_path : str, compiler : str, flags : list, globs):
        path = Path(dir_path)
        
        if path.is_dir():
            request = {
                "source" : [],
                "compilerSettings" : {
                    
                }
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
