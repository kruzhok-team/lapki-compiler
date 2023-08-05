import json
import base64
from datetime import datetime
from aiohttp import web
from aiofile import async_open
from aiopath import AsyncPath
import asyncjson
import aiohttp

try:
    from .GraphmlParser import GraphmlParser
    from .CJsonParser import CJsonParser
    from .fullgraphmlparser.graphml_to_cpp import CppFileWriter
    from .Compiler import Compiler
    from .JsonConverter import JsonConverter
    from .RequestError import RequestError
    from .config import BUILD_DIRECTORY
    from .wrapper import to_async
    from .Logger import Logger

except ImportError:
    from compiler.GraphmlParser import GraphmlParser
    from compiler.CJsonParser import CJsonParser
    from compiler.fullgraphmlparser.graphml_to_cpp import CppFileWriter
    from compiler.Compiler import Compiler
    from compiler.JsonConverter import JsonConverter
    from compiler.RequestError import RequestError
    from compiler.config import BUILD_DIRECTORY
    from compiler.wrapper import to_async
    from compiler.Logger import Logger


class Handler:
    def __init__(self):
        pass

    @staticmethod
    async def readSourceFile(filename: str, extension: str, path: str) -> dict[str, str]:
        async with async_open(f"{path}{filename}.{extension}", "r") as f:
            data = await f.read()
        return {
            "filename": filename,
            "extension": extension,
            "fileContent": data
        }

    @staticmethod
    async def main(request): 
        ws = web.WebSocketResponse(autoclose=False)
        await ws.prepare(request)
        await Logger.logger.info(request)
        async for msg in ws:
            await Logger.logger.info(msg)
            if msg.type == aiohttp.WSMsgType.TEXT:
                match msg.data:
                    case 'close':
                        await ws.close()
                    case 'arduino':
                        await Handler.handle_ws_compile(request, ws)
                    case 'importBerloga':
                        await Handler.handle_berloga_import(request, ws)
                    case 'exportBerloga':
                        await Handler.handle_berloga_export(request, ws)
            elif msg.type == aiohttp.WSMsgType.ERROR:
                print('ws connection closed with exception %s' %
                      ws.exception())

        return ws

    @staticmethod
    async def handle_ws_compile(request, ws=None):
        if ws is None:
            ws = web.WebSocketResponse(autoclose=False)
            await ws.prepare(request)
        try:
            await Logger.logger.info(request)
            data = json.loads(await ws.receive_str())
            await Logger.logger.info(data)
            compiler_settings = data["compilerSettings"]
            compiler = compiler_settings["compiler"]
            filename = compiler_settings["filename"][0].lower() + \
                compiler_settings["filename"][1:]
            flags = compiler_settings["flags"]
            dirname = str(datetime.now()) + '/'
            path = BUILD_DIRECTORY + dirname
            extension = Compiler.supported_compilers[compiler]["extension"][0]
            match compiler:
                case "g++" | "gcc":
                    await AsyncPath(path).mkdir(parents=True)
                    sm = await CJsonParser.parseStateMachine(data, ws,
                                                             filename=filename,
                                                             compiler=compiler,
                                                             path=path)
                    write_to_cpp_file = to_async(CppFileWriter(sm_name=filename, start_node=sm["startNode"],
                                                               start_action="", states=sm["states"],
                                                               notes=sm["notes"],
                                                               player_signal=sm["playerSignals"]).write_to_file)
                    await Logger.logger.info("Parsed and wrote to cpp")
                    await write_to_cpp_file(path, extension)
                    components = await CJsonParser.getComponents(data["components"])
                    libraries = await CJsonParser.getLibraries(components)
                    libraries = [*libraries, *Compiler.c_default_libraries]
                    build_files = await Compiler.getBuildFiles(libraries=libraries, compiler=compiler, directory=path)
                    await Compiler.includeLibraryFiles(libraries, dirname, ".h")
                    await Logger.logger.info(f"{libraries} included")
                case "arduino-cli":
                    dirname += filename + "/"
                    path += filename + "/"
                    await AsyncPath(path).mkdir(parents=True)
                    sm = await CJsonParser.parseStateMachine(data, ws, filename=filename, compiler=compiler, path=f"{path}{filename}.ino")
                    write_to_cpp_file = to_async(CppFileWriter(sm_name=filename, start_node=sm["startNode"], start_action="",
                                                      states=sm["states"], notes=sm["notes"],player_signal=sm["playerSignals"]).write_to_file)
                    await write_to_cpp_file(path, "ino")
                    await Logger.logger.info("Parsed and wrote to ino")
                    components = await CJsonParser.getComponents(data["components"])
                    libraries = await CJsonParser.getLibraries(components)
                    build_files = await Compiler.getBuildFiles(libraries=libraries, compiler=compiler, directory=path)
                    await Compiler.includeLibraryFiles([*libraries, *Compiler.c_default_libraries], dirname, ".h")
                    await Compiler.includeLibraryFiles(libraries, dirname, ".ino")
                    await Compiler.includeLibraryFiles(
                                                Compiler.c_default_libraries,
                                                dirname,
                                                ".c")
                    await Logger.logger.info(f"{libraries} included")
                case _:
                    await Logger.logger.info(f"Unsupported compiler {compiler}")
                    await RequestError(f"Unsupported compiler {compiler}. \
                        Supported compilers: {Compiler.supported_compilers.keys()}").dropConnection(ws)
                    return ws

            result = await Compiler.compile(base_dir=path, build_files=build_files, flags=flags, compiler=compiler)
            response = {
                "result": "NOTOK",
                "return code": result.return_code,
                "stdout": result.stdout,
                "stderr": result.stderr,
                "binary": [],
                "source": []
            }

            if result.return_code == 0:
                response["result"] = "OK"
                build_path = ''.join([BUILD_DIRECTORY, dirname, "build/"])
                source_path = ''.join([BUILD_DIRECTORY, dirname])
                async for path in AsyncPath(build_path).rglob("*"):
                    if await path.is_file():
                        async with async_open(path, 'rb') as f:
                            binary = await f.read()
                            fileinfo = {}
                            fileinfo["filename"] = path.name
                            b64_data = base64.b64encode(binary)
                            fileinfo["fileContent"] = b64_data.decode("ascii")
                            response["binary"].append(fileinfo)

                response["source"].append(await Handler.readSourceFile(filename, extension, source_path))
                response["source"].append(await Handler.readSourceFile(filename, "h", source_path))  
            await Logger.logger.info(f"Response: {response['result'], response['return code'], response['stdout'], response['stderr'], len(response['binary'])}")
            response = await asyncjson.dumps(response)
            await ws.send_json(response)
        except KeyError as e:
            await Logger.logger.error(f"Invalid request, there isn't '{e.args[0]}' key.")
            await RequestError(f"Invalid request, there isn't '{e.args[0]}' key.").dropConnection(ws)
            await ws.close()
            return ws
        except Exception:
            await Logger.logException()
            await RequestError("Something went wrong").dropConnection(ws)
            await ws.close()
            return ws
        return ws

    @staticmethod
    async def handle_ws_compile_source(request):
        ws = web.WebSocketResponse()
        await ws.prepare(request)    
        data = json.loads(await ws.receive_json())
        await Logger.logger.info(data)
        try:
            source = data["source"]
            flags = data["compilerSettings"]["flags"]
            compiler = data["compilerSettings"]["compiler"]

        except KeyError as e:
            await RequestError(f"Invalid request, there isn't key {e.args[0]}").dropConnection(ws)
        if compiler not in Compiler.supported_compilers:
            await Logger.logger.error(f"Unsupported compiler {compiler}.")
            await RequestError(f"Unsupported compiler {compiler}.\
                Supported compilers: {Compiler.supported_compilers.keys()}").dropConnection(ws)

        dirname = BUILD_DIRECTORY + str(datetime.now()) + '/'

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
                "result": "OK",
                "return code": result.return_code,
                "stdout": result.stdout,
                "stderr": result.stderr,
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
    async def handle_berloga_import(request, ws=None):
        if ws is None:
            ws = web.WebSocketResponse()
            await ws.prepare(request)
        unprocessed_xml = await ws.receive_str()
        await Logger.logger.info("XML received!")
        try:
            response = await GraphmlParser.parse(unprocessed_xml)
        except KeyError as e:
            await Logger.logException()
            await RequestError(f"There isn't key {e.args[0]}").dropConnection(ws)
            return ws
        except Exception:
            await Logger.logException()
            await RequestError("Something went wrong!").dropConnection(ws)
            return ws
        await Logger.logger.info("Converted!")
        await ws.send_json(response)

        return ws

    @staticmethod
    async def handle_berloga_export(request, ws=None):
        if ws is None:
            ws = web.WebSocketResponse()
            await ws.prepare(request)
        schema = json.loads(await ws.receive_json())
        await Logger.logger.info(schema)
        try:
            sm = await CJsonParser.parseStateMachine(schema, ws=ws, compiler="Berloga")
            converter = JsonConverter(ws)
            xml = await converter.parse(sm["states"], sm["startNode"])
        except KeyError as e:
            await Logger.logException()
            await RequestError(f"There isn't key {e.args[0]}").dropConnection(ws)
            return ws
        await Logger.logger.info("Converted!")
        await ws.send_str(xml)

        return ws

    @staticmethod
    async def handle_get_compile(request):
        return web.Response(text="Hello world!")
