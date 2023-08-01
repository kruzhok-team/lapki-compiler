import asyncio
import pytest
from compiler.Client import Client
from aiofile import async_open
import base64
from time import gmtime, strftime
import json
from pathlib import Path
import subprocess

from compiler.config import SERVER_PORT

pytest_plugins = ('pytest_asyncio',)

BASE_ADDR = f'http://localhost:{SERVER_PORT}/ws'


@pytest.mark.asyncio
async def test_sendSmth():
    client = Client()
    await client.doConnect(BASE_ADDR)
    await client.sendSMJson(path='examples/ExampleRequestSM.json')


@pytest.mark.asyncio
async def test_sendMultifileProject():
    client = Client()
    await client.doConnect(f'{BASE_ADDR}/source')
    req = await client.sendMultiFileProject("examples/MultifilesExample", "g++", ["-std=c++2a"], ["*.cpp", "*.hpp"])

    result = await client.ws.receive_json()
    result = json.loads(result)
    path = "client/" + strftime('%Y-%m-%d %H:%M:%S', gmtime()) + "/"
    Path(path).mkdir(parents=True)
    for binary in result["binary"]:
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])

        async with async_open(path + binary["filename"], "wb") as f:
            await f.write(data)
    
    
    subprocess.run(["chmod", "u+x", path + "a.out"])
    result = subprocess.run(["./" + path + "a.out"], capture_output=True, text=True)
    stdout = result.stdout.replace("\n", '')
    assert stdout == "Result: 8"


@pytest.mark.asyncio
async def test_sendArduino():
    client = Client()
    await client.doConnect(f'{BASE_ADDR}/source')
    req = await client.sendMultiFileProject("examples/ExampleSketch", "arduino-cli", ["-b", "arduino:avr:uno", "ExampleSketch.ino"], ["*.ino"])
    result = await client.ws.receive_json()
    result = json.loads(result)
    path = "client/" + strftime('%Y-%m-%d %H:%M:%S', gmtime()) + "/"
    Path(path).mkdir(parents=True)
    for binary in result["binary"]:
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(path + binary["filename"], "wb") as f:
            await f.write(data)


@pytest.mark.asyncio
async def test_sendSMJson():
    client = Client()
    await client.doConnect(BASE_ADDR)
    await client.sendSMJson("examples/ExampleRequestSM5.json")
    response = json.loads(await client.ws.receive_json())


@pytest.mark.asyncio
async def test_sendNestedSMJson():
    client = Client()
    await client.doConnect(BASE_ADDR)
    await client.sendSMJson("examples/ExampleRequestSMWithChilds.json")
    response = json.loads(await client.ws.receive_json())


@pytest.mark.asyncio
async def test_sendArduinoSMJson():
    client = Client()
    await client.doConnect(BASE_ADDR)
    await client.sendSMJson("examples/ExampleRequestSMArduino.json")
    response = json.loads(await client.ws.receive_json())
    path = "client/" + strftime('%Y-%m-%d %H:%M:%S', gmtime()) + "/"
    Path(path).mkdir(parents=True)
    for binary in response["binary"]:
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(path + binary["filename"], "wb") as f:
            await f.write(data)


@pytest.mark.asyncio
async def test_berlogaImport():
    client = Client()
    await client.doConnect(f"{BASE_ADDR}/berloga/import")
    response = await client.importBerlogaScheme("examples/NewAutoborder.graphml")
    path = "client/" + strftime('%Y-%m-%d %H:%M:%S', gmtime()) + "/"
    Path(path).mkdir(parents=True)
    async with async_open(path + "berlogaScheme.json", "w") as f:
        await f.write(response)
    
    await client.ws.close()

@pytest.mark.asyncio
async def test_berlogaExport():
    client = Client()
    await client.doConnect(f"{BASE_ADDR}/berloga/export")
    response = await client.exportBerlogaScheme("compiler/schemas/berlogaScheme.json")
    path = "client/" + strftime('%Y-%m-%d %H:%M:%S', gmtime()) + "/"
    Path(path).mkdir(parents=True)
    async with async_open(path + "berlogaScheme.xml", "w") as f:
        await f.write(response)
    
    await client.ws.close()

@pytest.mark.asyncio
async def test_sendTestSchema():
    client = Client()
    await client.doConnect(BASE_ADDR)
    await client.sendSMJson("examples/testSchema.json")
    response = json.loads(await client.ws.receive_json())
    path = "client/" + strftime('%Y-%m-%d %H:%M:%S', gmtime()) + "/"
    Path(path).mkdir(parents=True)
    for binary in response["binary"]:
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(path + binary["filename"], "wb") as f:
            await f.write(data)

@pytest.mark.asyncio
async def test_sendSchemaWithId():
    client = Client()
    await client.doConnect(BASE_ADDR)
    await client.sendSMJson("examples/schemaWithId.json")
    response = json.loads(await client.ws.receive_json())
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    for binary in response["binary"]:
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])