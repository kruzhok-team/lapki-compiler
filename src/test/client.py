import sys
sys.path.append('.')
sys.path.append('./src/compiler')
sys.path.append('./src/compiler/fullgraphmlparser')
import asyncio
from Client import Client

async def test_sendSmth():
    client = Client()
    await client.doConnect('http://localhost:8080/ws')
    await client.sendSMJson(path='src/test/Examples/ExampleRequestSM5.json')

asyncio.run(test_sendSmth())