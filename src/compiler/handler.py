from aiohttp import web
from JsonParser import JsonParser
from Compiler import Compiler, CompilerResult
from JsonConverter import JsonConverter
from RequestError import RequestError
import aiohttp
from aiofile import async_open
from time import gmtime, strftime
import json
import base64
from pathlib import Path
import JsonParser
class Handler:
    base_dir = "server/"
    def __init__():
        pass
    
    @staticmethod
    async def handle_ws_compile(request):
        ws = web.WebSocketResponse()
        await ws.prepare(request)
        
        #TODO Прикрутить logger
        #TODO Вынести в JSON Parser
        data = json.loads(await ws.receive_json())
        
        try:
            source = data["source"]
            compiler = data["compiler settings"]["compiler"]
            flags = data["compiler settings"]["flags"]
            
            #Как генерировать названия файлов? По времени? По токену?
            #TODO Вынести работу с файлами в FileManager?
            
            filename = strftime('%Y-%m-%d %H:%M:%S', gmtime())
            
            try:
                fileformat = Compiler.supported_compilers[compiler]
            except KeyError:
                await RequestError(F"Unsupported compiler {compiler}. Supported compilers: {Compiler.supported_compilers.keys()}").dropConnection(ws)
            
            Path(f"server/{filename}/").mkdir(parents=True, exist_ok=True)
            
            async with async_open(f"server/{filename}/{filename}.{fileformat}", 'w') as f:
                await f.write(source)

            result = await Compiler.compile(f"server/{filename}/", filename, flags, compiler)
            
            
            response = {
                "result" : "OK",
                "return code" : result.return_code,
                "stdout" : result.stdout,
                "stderr" : result.stderr,
                "binary" : ""
            }
            if result.return_code != 0:
                response = json.dumps(response)
                await ws.send_json(response)
            else:
                async with async_open(f"server/{filename}/{filename}.zip", 'rb') as f:
                    binary_data = await f.read()
                
                b64_data = base64.b64encode(binary_data)
                response["binary"] = b64_data.decode("ascii")
            
            response = json.dumps(response)
            await ws.send_json(response)
            
        except KeyError:
            await RequestError("Invalid request").dropConnection(ws)
        
        await ws.close()
        
        return ws
    
    @staticmethod
    async def handle_ws_compile_source(request):
        ws = web.WebSocketResponse()
        await ws.prepare(request)
        
        data = json.loads(await ws.receive_json())
        
        try:
            source = data["source"]
            compiler = data["compilerSettings"]["compiler"]
            flags = data["compilerSettings"]["flags"]
        except KeyError:
            await RequestError(F"Unsupported compiler {compiler}. Supported compilers: {Compiler.supported_compilers.keys()}").dropConnection(ws)
        
        dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
        Path(Handler.base_dir + dirname).mkdir(parents=True, exist_ok=True)
        files = await JsonParser.getFiles(data)
        for file in files:
            path = ''.join([Handler.base_dir, dirname, file.name, file.extension])
            print(path)
            async with async_open(path, 'w') as f:
                await f.write(file.content)
    @staticmethod
    async def handle_get_compile(request):
        return web.Response(text="Hello world!")