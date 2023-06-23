import asyncio
import pytest
from compiler.Client import Client
from aiofile import async_open
import base64
pytest_plugins = ('pytest_asyncio',)

@pytest.mark.asyncio
async def test_sendSmth():
    client = Client()
    await client.doConnect('http://localhost:8080/ws')
    await client.sendSMJson(path='src/test/Examples/ExampleRequest.json')

@pytest.mark.asyncio
async def test_sendSourceFileCpp():
    client = Client()
    await client.doConnect('http://localhost:8080/ws/source')
    await client.sendSourceFile(path='src/test/Examples/cpp_example.cpp', compiler="g++", flags=["-o"])
    
    response = await client.getResult()
    
    async with async_open("src/test/Examples/cpp_example.o", 'rb') as f:
        binary = await f.read()
    
    binary = base64.b64encode(binary)
    binary = binary.decode("ascii")
    
    print(response)    
    assert response == {
        "result" : "OK",
        "compilationCommand" : "g++ cpp_example.cpp -o build/cpp_example.o",
        "returnCode" : 0,
        "stdout" : "",
        "stderr" : "",
        "binary" : {
            "filename": "cpp_example.o",
            "fileContent" : binary 
        }
    }

@pytest.mark.asyncio
async def test_sendMultifileProject():
    client = Client()
    await client.doConnect('http://localhost:8080/ws/source')
    req = await client.sendMultiFileProject("src/test/Examples/MultifilesExample", "g++", "-o", ("*.cpp", "*.hpp"))

    print(req)
    
    
    
    
@pytest.mark.asyncio
async def test_sendSMJson():
    client = Client()
    await client.doConnect('http://localhost:8080/ws')
    await client.sendSMJson("src/test/Examples/ExampleRequest.json")
    
    pass