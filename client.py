import aiohttp
import asyncio
import json
from aiofile import async_open
import subprocess
import base64
from time import gmtime, strftime
from pathlib import Path
from aioshutil import unpack_archive
#TODO Вынести в отдельный класс
#TODO Написать тесты
async def main():
    session = aiohttp.ClientSession()
    
    async with session.ws_connect('http://localhost:8080/ws') as ws:
        # with open('src/Examples/cpp_example.cpp', 'r') as inp:
        #     data = inp.read()
        
        # await ws.send_json({"source" : data, "compiler settings": {"compiler" : "g++", "flags" : ["-o"]}})
        # async with async_open('src/Examples/ExampleSketch/ExampleSketch.ino', 'r') as inp:
        #     data = await inp.read()
        
        async with async_open('src/Examples/ExampleRequest4.json', 'r') as req:
            json_data = json.loads(await req.read())
        
        await ws.send_json(json_data)
        output = await ws.receive_json()
        json_data = json.loads(output)
        result = json_data["result"]
        
        if result == 'OK':
            print(result)
            for k in list(json_data.keys())[:-1]:
                print(f"{k}: {json_data[k]}")
            binary = json_data["binary"]
            binary = binary.encode('ascii')
            binary = base64.b64decode(binary)
            
            filename = strftime('%Y-%m-%d %H:%M:%S', gmtime())
            async with async_open(f"client/{filename}.zip", 'wb') as f:
                await f.write(binary)
        else:
            print(result)

Path("client/").mkdir(parents=True, exist_ok=True)
asyncio.run(main())