"""Legacy tests the parsing Lapki IDE's JSON schemes\
    and import/export BearlogaDefend."""
import json
from time import gmtime, strftime
import random
import base64
from pathlib import Path

import pytest
from compiler.Client import Client
from aiofile import async_open
from compiler.config import get_config

pytest_plugins = ('pytest_asyncio',)

BASE_ADDR = f'http://localhost:{get_config().server_port}/ws'


def getPath():
    """Get path to client directory."""
    return ('client/' +
            strftime('%Y-%m-%d %H:%M:%S', gmtime()) +
            str(random.randint(0, 10000))
            )


@pytest.mark.asyncio
async def test_berlogaImport():
    """Test generating JSON from Bearloga schemes."""
    client = Client()
    await client.doConnect(f'{BASE_ADDR}/berloga/import')
    response = await client.importBerlogaScheme(
        'examples/old/bearlogaSchemas/'
        'Autoborder_638330223036439120.graphml',
        'Autoborder_12314124')
    print(response)
    path = 'client/' + strftime('%Y-%m-%d %H:%M:%S',
                                gmtime()) + str(random.randint(0, 10000)) + '/'
    Path(path).mkdir(parents=True)
    async with async_open(path + 'berlogaScheme.json', 'w') as f:
        await f.write(json.dumps(response['source'][0]['fileContent'],
                                 indent=4,
                                 ensure_ascii=False))

    await client.ws.close()

    assert response['source'][0]['fileContent'] is not None and\
        response['source'][0]['fileContent'] != 'null'


@pytest.mark.asyncio
async def test_berlogaExport():
    """Test generating Bearloga schemes from JSON."""
    client = Client()
    await client.doConnect(f'{BASE_ADDR}/berloga/export')
    response = await client.exportBerlogaScheme('examples/old/bearlogaSchemas/'
                                                'berlogaScheme.json')
    path = 'client/' + strftime('%Y-%m-%d %H:%M:%S',
                                gmtime()) + str(random.randint(0, 10000)) + '/'
    Path(path).mkdir(parents=True)
    async with async_open(path + 'berlogaScheme.graphml', 'w') as f:
        await f.write(response)

    await client.ws.close()

    assert response is not None and response != ''


@pytest.mark.parametrize('path_to_scheme',
                         [
                             pytest.param('examples/old/testTimer.json',
                                          id='Timer'),
                             pytest.param('examples/old/testCounter.json',
                                          id='Counter'),
                             pytest.param('examples/old/testSerial.json',
                                          id='Serial'),
                             pytest.param('examples/old/testSerial2.json',
                                          id='Serial2'),
                             pytest.param('examples/old/testPWM.json',
                                          id='PWM'),
                             pytest.param('examples/old/testDigitalOut.json',
                                          id='DigitalOut'),
                             pytest.param('examples/old/testDigitalIn.json',
                                          id='DigitalIn'),
                             pytest.param('examples/old/arduino-blinker.json',
                                          marks=pytest.mark.xfail,
                                          id='Arduino blinker'),
                             pytest.param('examples/old/testAnalogIn.json',
                                          id='AnalogIn'),
                             pytest.param('examples/old/testAnalogOut.json',
                                          id='AnalogOut'),
                             pytest.param('examples/old/testShiftOut.json',
                                          id='ShiftOut'),
                         ]
                         )
# @pytest.mark.skip('Test manually with startup module!')
@pytest.mark.asyncio
async def test_compile_json_scheme(path_to_scheme: str):
    """
    Test code generating from Lapki IDE json scheme.

    Module startup is required.
    """
    client = Client()
    await client.doConnect(BASE_ADDR)
    response = await client.sendSMJson(path_to_scheme)
    print(response)
    path = getPath()
    build_path = path + '/build/'
    source_path = path + '/source/'
    Path(build_path).mkdir(parents=True)
    Path(source_path).mkdir(parents=True)
    count_binary = 0
    count_source = 0
    for binary in response['binary']:
        count_binary += 1
        data = binary['fileContent'].encode('ascii')
        data = base64.b64decode(binary['fileContent'])
        async with async_open(build_path + binary['filename'], 'wb') as f:
            await f.write(data)

    for source in response['source']:
        count_source += 1
        async with async_open(source_path +
                              source['filename'] +
                              '.' +
                              source['extension'],
                              'w') as f:
            await f.write(source['fileContent'])

    assert count_binary > 0 and count_source > 0
