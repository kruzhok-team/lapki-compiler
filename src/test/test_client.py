import asyncio
import pytest
from .Client import Client
pytest_plugins = ('pytest_asyncio',)

@pytest.mark.asyncio
async def test_sendJson():
    client = Client()
    await client.doConnect('http://localhost:8080/ws')
    await client.sendJson(path='src/test/Examples/ExampleRequest.json')

@pytest.mark.asyncio
async def test_cpp():
    client = Client()
    await client.doConnect('http://localhost:8080/ws')
    await client.sendJson(path='src/test/Examples/ExampleRequest.json')
    
    json_data = await client.getResult()
    
    print(json_data)
    
    