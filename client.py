import aiohttp
import asyncio
import json
from aiofile import async_open
import subprocess
import base64

#TODO Вынести в отдельный класс
#TODO Написать тесты
async def main():
    session = aiohttp.ClientSession()
    
    
    async with session.ws_connect('http://localhost:8080/ws') as ws:
        with open('src/cpp_example.cpp', 'r') as inp:
            data = inp.read()
        
        await ws.send_json({"source" : data, "compiler settings": {"compiler" : "g++", "flags" : ["-o"]}})
        
        output = await ws.receive_json()
        json_data = json.loads(output)
        binary = json_data["binary"]
        binary = binary.encode('ascii')
        binary = base64.b64decode(binary)
        async with async_open("./biba.o", 'wb') as f:
            await f.write(binary)
            
        subprocess.run("chmod u+x biba.o", shell=True)
        output = subprocess.run("./biba.o", shell=True)
        print(output)
        async for msg in ws:
            if msg.type == aiohttp.WSMsgType.TEXT:
                print(msg)
            elif msg.type == aiohttp.WSMsgType.ERROR:
                break
            else:
                pass
                
    
asyncio.run(main())