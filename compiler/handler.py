from aiohttp import web
from Compiler import Compiler, CompilerResult
from JsonConverter import JsonConverter
from RequestError import RequestError
from wrapper import to_async
from aiofile import async_open
from time import gmtime, strftime
import json
import base64
from aiopath import AsyncPath
from CJsonParser import CJsonParser
from fullgraphmlparser.graphml_to_cpp import CppFileWriter
from config import BASE_DIRECTORY
import asyncjson

class Handler: 
    def __init__():
        pass

    # TODO
    @staticmethod
    async def handle_ws_compile(request):
        ws = web.WebSocketResponse()
        await ws.prepare(request)

        # TODO Прикрутить logger
        data = json.loads(await ws.receive_json())
        try:
            
            compiler_settings = data["compilerSettings"]
            
            compiler = compiler_settings["compiler"]
            filename = compiler_settings["filename"][0].lower() + compiler_settings["filename"][1:]
            flags = compiler_settings["flags"]
            dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime()) + '/'
            path = BASE_DIRECTORY + dirname
            match compiler:
                case "g++" | "gcc":
                    await AsyncPath(path).mkdir(parents=True)
                    sm = await CJsonParser.parseStateMachine(data, filename=filename, compiler=compiler, path=path)
                    cpp_file = to_async(CppFileWriter(sm_name=filename, start_node=sm["startNode"], start_action="", states=sm["states"], notes=sm["notes"],player_signal=sm["playerSignals"]).write_to_file)
                    await cpp_file(path, "cpp")
                    components = await CJsonParser.getComponents(data["components"])
                    libraries = await CJsonParser.getLibraries(components)
                    libraries = [*libraries, *Compiler.c_default_libraries]
                    build_files = await Compiler.getBuildFiles(libraries=libraries, compiler=compiler, directory=path)
                    await Compiler.includeLibraryFiles(libraries, dirname, ".h")
                    
                case "arduino-cli":
                    dirname += filename + "/"
                    path += filename + "/"
                    await AsyncPath(path).mkdir(parents=True)
                    sm = await CJsonParser.parseStateMachine(data, filename=filename, compiler=compiler, path=f"{path}{filename}.ino")
                    cpp_file = to_async(CppFileWriter(sm_name=filename, start_node=sm["startNode"], start_action="", 
                                                      states=sm["states"], notes=sm["notes"],player_signal=sm["playerSignals"]).write_to_file)
                    await cpp_file(path, "ino")
                    components = await CJsonParser.getComponents(data["components"])
                    libraries = await CJsonParser.getLibraries(components)
                    build_files = await Compiler.getBuildFiles(libraries=libraries, compiler=compiler, directory=path)
                    await Compiler.includeLibraryFiles([*libraries, *Compiler.c_default_libraries], dirname, ".h")
                    await Compiler.includeLibraryFiles(libraries, dirname, ".ino")
                    await Compiler.includeLibraryFiles(Compiler.c_default_libraries, dirname, ".c")
                case _:
                    print("here")
                    await RequestError(f"Unsupported compiler {compiler}. Supported compilers: {Compiler.supported_compilers.keys()}").dropConnection(ws)
            
            result = await Compiler.compile(base_dir=path, build_files=build_files, flags=flags, compiler=compiler)
            response = {
                "result" : "OK",
                "return code" : result.return_code,
                "stdout" : result.stdout,
                "stderr" : result.stderr,
                "binary" : []
            }
            if result.return_code == 0:
                async for path in AsyncPath(''.join([BASE_DIRECTORY, dirname, "build/"])).rglob("*"):
                    if await path.is_file():
                        async with async_open(path, 'rb') as f:
                            binary = await f.read()
                            fileinfo = {}
                            fileinfo["filename"] = path.name
                            b64_data = base64.b64encode(binary)
                            fileinfo["fileContent"] = b64_data.decode("ascii")
                            response["binary"].append(fileinfo)
            
            response = await asyncjson.dumps(response)
            await ws.send_json(response)
            
        except KeyError:
            await RequestError(f"Invalid request, there isn't '{KeyError.args[0]}' key.").dropConnection(ws)
        
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
        dirname = BASE_DIRECTORY + strftime('%Y-%m-%d %H:%M:%S', gmtime()) + '/'
        
        if compiler == "arduino-cli":
            dirname += source[0]["filename"] + "/"
        
        await AsyncPath(dirname).mkdir(parents=True, exist_ok=True)
        files = await CJsonParser.getFiles(source)
        for file in files:
            path = ''.join([dirname, file.name, file.extension])
            async with async_open(path, 'w') as f:
                await f.write(file.content)
        
        build_files = await Compiler.getBuildFiles(libraries=[], compiler=compiler, directory=dirname)
        result = await Compiler.compile(base_dir=dirname, build_files=build_files, flags=flags, compiler=compiler)
        response = {
                "result" : "OK",
                "return code" : result.return_code,
                "stdout" : result.stdout,
                "stderr" : result.stderr,
                "binary": []
            }
        
        async for path in AsyncPath(''.join([dirname, "/build/"])).rglob("*"):
            if await path.is_file():
                async with async_open(path, 'rb') as f:
                    binary = await f.read()
                    fileinfo = {}
                    fileinfo["filename"] = path.name
                    b64_data = base64.b64encode(binary)
                    fileinfo["fileContent"] = b64_data.decode("ascii")
                    response["binary"].append(fileinfo)
        
        await ws.send_json(await asyncjson.dumps(response))     
        await ws.close()
        
        return ws
    @staticmethod
    async def handle_get_compile(request):
        return web.Response(text="Hello world!")