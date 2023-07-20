import sys
import asyncio
import json

sys.path.append('.')
sys.path.append('./compiler')
sys.path.append('./compiler/fullgraphmlparser')

from Client import Client
from compiler.config import SERVER_PORT


async def test_sendSmth():
    client = Client()
    await client.doConnect(f'http://localhost:{SERVER_PORT}/ws')
    await client.sendSMJson(path='examples/ExampleRequestSMArduino.json')
    response = json.loads(await client.ws.receive_json())
    print(response)

asyncio.run(test_sendSmth())
