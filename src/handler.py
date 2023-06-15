from aiohttp import web
from JsonParser import JsonParser
from Compiler import Compiler
from JsonConverter import JsonConverter
import aiohttp
from aiofile import async_open
from time import gmtime, strftime
import json

class Handler:
    def __init__():
        pass
    
    @staticmethod
    async def handle_ws_compile(request):
        ws = web.WebSocketResponse()
        await ws.prepare(request)
        
        
        #TODO Прикрутить logger
        
        #TODO Вынести в JSON Parser
        data = await ws.receive_json()
        
        try:
            source = data["source"]
            compiler = data["compiler settings"]["compiler"]
            flags = data["compiler settings"]["flags"]
            
            #Как генерировать названия файлов? По времени? По токену?
            #TODO Вынести работу с файлами в FileManager?
            filename = strftime('%Y-%m-%d %H:%M:%S', gmtime())
            async with async_open(f"{filename}.cpp", 'w') as f:
                await f.write(source)

            output = await Compiler.compile(f"{filename}.cpp", f"{filename}.o", flags, compiler)
            
            async with async_open(f"{filename}.o", 'rb') as f:
                binary_data = await f.read()
                
                await ws.send_bytes(binary_data)
            
            
        except KeyError:
            print("Invalid data")
        
        async for msg in ws:
            if msg.type == aiohttp.WSMsgType.TEXT:
                print(msg.data)
            elif msg.type == aiohttp.WSMsgType.ERROR:
                print('ws connection closed with exception %s' %
                    ws.exception())
            else:
                print(msg.data)
        
        return ws
    
    @staticmethod
    async def handle_get_compile(request):
        return web.Response(text="Hello world!")