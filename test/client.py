import sys
sys.path.append('.')
sys.path.append('./compiler')
sys.path.append('./compiler/fullgraphmlparser')
import asyncio
from Client import Client

from compiler.config import SERVER_PORT

async def test_sendSmth():
    client = Client()
    await client.doConnect(f'http://localhost:{SERVER_PORT}/ws')
    await client.sendSMJson(path='examples/ExampleRequestSM5.json')

asyncio.run(test_sendSmth())