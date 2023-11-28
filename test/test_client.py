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
    path = "client/" + strftime('%Y-%m-%d %H:%M:%S', gmtime()) + "/"
    Path(path).mkdir(parents=True)
    for binary in result["binary"]:
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])

        async with async_open(path + binary["filename"], "wb") as f:
            await f.write(data)

    subprocess.run(["chmod", "u+x", path + "a.out"])
    result = subprocess.run(["./" + path + "a.out"],
                            capture_output=True, text=True)
    stdout = result.stdout.replace("\n", '')
    assert stdout == "Result: 8"


@pytest.mark.asyncio
async def test_sendArduino():
    client = Client()
    await client.doConnect(f'{BASE_ADDR}/source')
    req = await client.sendMultiFileProject("../examples/ExampleSketch/", "arduino-cli", ["-b", "arduino:avr:uno", "ExampleSketch.ino"], ["*.ino"])
    result = await client.ws.receive_json()
    path = "client/" + strftime('%Y-%m-%d %H:%M:%S', gmtime()) + "/"
    Path(path).mkdir(parents=True)

    count = 0

    for binary in result["binary"]:
        count += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(path + binary["filename"], "wb") as f:
            await f.write(data)

    assert count != 0


@pytest.mark.asyncio
async def test_berlogaImport():
    client = Client()
    await client.doConnect(f"{BASE_ADDR}/berloga/import")
    # response = await client.importBerlogaScheme("compiler/schemas/Autoborder_with_actions.graphml")
    response = await client.importBerlogaScheme("examples/bearlogaSchemas/Autoborder_638330223036439120.graphml", "Autoborder_12314124")
    print(response)
    path = "client/" + strftime('%Y-%m-%d %H:%M:%S', gmtime()) + "/"
    Path(path).mkdir(parents=True)
    async with async_open(path + "berlogaScheme.json", "w") as f:
        await f.write(json.dumps(response["source"][0]["fileContent"], indent=4, ensure_ascii=False))

    await client.ws.close()

    assert response["source"][0]["fileContent"] != None and response["source"][0]["fileContent"] != "null"


@pytest.mark.asyncio
async def test_berlogaExport():
    client = Client()
    await client.doConnect(f"{BASE_ADDR}/berloga/export")
    response = await client.exportBerlogaScheme("compiler/schemas/berlogaScheme.json")
    path = "client/" + strftime('%Y-%m-%d %H:%M:%S', gmtime()) + "/"
    Path(path).mkdir(parents=True)
    async with async_open(path + "berlogaScheme.graphml", "w") as f:
        await f.write(response)

    await client.ws.close()

    assert response != None and response != ''


@pytest.mark.asyncio
async def test_sendSchemaWithId():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/schemaWithId(actual).json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0


@pytest.mark.asyncio
async def test_timerSchema():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/testTimer.json")
    print(response)
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


@pytest.mark.asyncio
async def test_counterSchema():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/testCounter.json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0


@pytest.mark.asyncio
async def test_Serial():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/testSerial.json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0


@pytest.mark.asyncio
async def test_Serial2():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/testSerial2.json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0


@pytest.mark.asyncio
async def test_PWM():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/testPWM.json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0


@pytest.mark.asyncio
async def test_digitalOut():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/testDigitalOut.json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0


@pytest.mark.asyncio
async def test_digitalIn():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/testDigitalIn.json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0


@pytest.mark.asyncio
async def test_blinker():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/arduino-blinker.json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0


@pytest.mark.asyncio
async def test_analogOut():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/testAnalogOut.json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0


@pytest.mark.asyncio
async def test_analogIn():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/testAnalogIn.json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0


@pytest.mark.asyncio
async def test_ShiftOut():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/testShiftOut.json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0


@pytest.mark.asyncio
async def test_User():
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson("examples/testUser.json")
    print(response)
    dirname = strftime('%Y-%m-%d %H:%M:%S', gmtime())
    build_path = "client/" + dirname + "/build/"
    source_path = "client/" + dirname + "/source/"
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response["binary"]:
        count_binary += 1
        data = binary["fileContent"].encode('ascii')
        data = base64.b64decode(binary["fileContent"])
        async with async_open(build_path + binary["filename"], "wb") as f:
            await f.write(data)

    for source in response["source"]:
        count_source += 1
        async with async_open(source_path + source["filename"] + "." + source["extension"], "w") as f:
            await f.write(source["fileContent"])

    assert count_binary > 0 and count_source > 0
