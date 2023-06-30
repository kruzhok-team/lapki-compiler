from aiohttp import web
from Compiler import Compiler, CompilerResult
from JsonConverter import JsonConverter
from RequestError import RequestError
import aiohttp
from aiofile import async_open
from time import gmtime, strftime
import json
import base64
from pathlib import Path
from CJsonParser import CJsonParser
from fullgraphmlparser.graphml_to_cpp import CppFileWriter
from config import BASE_DIRECTORY
class Handler:
    def __init__():
        pass
    #TODO 
    @staticmethod
    async def handle_ws_compile(request):
        ws = web.WebSocketResponse()
        await ws.prepare(request)
        
        #TODO Прикрутить logger
        data = json.loads(await ws.receive_json())
        
        try:
            sm = await CJsonParser.parseStateMachine(data)
            
            compiler_settings = data["compilerSettings"]
            
            compiler = compiler_settings["compiler"]
            filename = compiler_settings["filename"][0].lower() + compiler_settings["filename"][1:]
            flags = compiler_settings["flags"]
            dirname = strftime('%Y_%m_%d_%H%_M_%S', gmtime()) + '/'
            
            path = Handler.base_dir + dirname
            Path(path).mkdir(parents=True, exist_ok=True)
            
        
            CppFileWriter(sm_name=filename, start_node=sm["startNode"], start_action="", states=sm["states"], notes=sm["notes"],player_signal=sm["playerSignals"]).write_to_file(path)
                
            if compiler not in Compiler.supported_compilers:
                await RequestError(f"Unsupported compiler {compiler}. Supported compilers: {Compiler.supported_compilers.keys()}").dropConnection(ws)
            
            component = await CJsonParser.getComponents(data["components"])
            await Compiler.includeLibraries()
            result = await Compiler.compile(base_dir=BASE_DIRECTORY+dirname, flags=flags, compiler=compiler)
            
            
            
            
            # async with async_open(f"server/{filename}/{filename}.{fileformat}", 'w') as f:
            #     await f.write(source)

            # result = await Compiler.compile(f"server/{filename}/", filename, flags, compiler)
            
            
            # response = {
            #     "result" : "OK",
            #     "return code" : result.return_code,
            #     "stdout" : result.stdout,
            #     "stderr" : result.stderr,
            #     "binary" : ""
            # }
            # if result.return_code != 0:
            #     response = json.dumps(response)
            #     await ws.send_json(response)
            # else:
            #     async with async_open(f"server/{filename}/{filename}.zip", 'rb') as f:
            #         binary_data = await f.read()
                
            #     b64_data = base64.b64encode(binary_data)
            #     response["binary"] = b64_data.decode("ascii")
            
            # response = json.dumps(response)
            # await ws.send_json(response)
            
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
            flags = data["compilerSettings"]["flags"]
            compiler = data["compilerSettings"]["compiler"]

        except KeyError:
            await RequestError(f"Invalid request").dropConnection(ws)
        
        if compiler not in Compiler.supported_compilers:
            await RequestError(f"Unsupported compiler {compiler}. Supported compilers: {Compiler.supported_compilers.keys()}").dropConnection(ws)
        dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime()) + '/'
        
        if compiler == "arduino-cli":
            dirname += source[0]["filename"] + "/"
        
        Path(BASE_DIRECTORY + dirname).mkdir(parents=True, exist_ok=True)
        files = await CJsonParser.getFiles(source)
        for file in files:
            path = ''.join([BASE_DIRECTORY, dirname, file.name, file.extension])
            async with async_open(path, 'w') as f:
                await f.write(file.content)
        
        result = await Compiler.compile(base_dir=BASE_DIRECTORY + dirname, flags=flags, compiler=compiler)
        response = {
                "result" : "OK",
                "return code" : result.return_code,
                "stdout" : result.stdout,
                "stderr" : result.stderr,
                "binary": []
            }
        
        for path in Path(''.join([BASE_DIRECTORY, dirname, "/build/"])).rglob("*"):
            if path.is_file():
                async with async_open(path, 'rb') as f:
                    binary = await f.read()
                    fileinfo = {}
                    fileinfo["filename"] = path.name
                    b64_data = base64.b64encode(binary)
                    fileinfo["fileContent"] = b64_data.decode("ascii")
                    response["binary"].append(fileinfo)
        
        await ws.send_json(json.dumps(response))     
        await ws.close()
        
        return ws
    @staticmethod
    async def handle_get_compile(request):
        return web.Response(text="Hello world!")