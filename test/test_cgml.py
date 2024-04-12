"""Module for testing CGML.py file."""
import os
import inspect
import json
import shutil
import time
from pathlib import Path
from contextlib import contextmanager

import pytest
from aiopath import AsyncPath
from compiler.config import BUILD_DIRECTORY
from compiler.handler import compile_xml, create_response
from cyberiadaml_py.cyberiadaml_parser import CGMLParser
from compiler.fullgraphmlparser.graphml_to_cpp import CppFileWriter
from compiler.types.inner_types import InnerEvent, InnerTrigger
from compiler.types.platform_types import Platform
from compiler.CGML import __parse_actions, parse
from compiler.PlatformManager import PlatformManager

pytest_plugins = ('pytest_asyncio',)


@pytest.fixture
async def init_platform():
    await PlatformManager.load_platform('compiler/platforms/Arduino.json')


@contextmanager
def create_test_folder(path: str, wait_time: int):
    try:
        Path(path).mkdir(parents=True)
        yield
    finally:
        time.sleep(wait_time)
        shutil.rmtree(path)


@pytest.mark.parametrize(
    'path',
    [
        pytest.param('examples/CyberiadaFormat-Autoborder.graphml',
                     id='parse AutoBorder'),
        pytest.param('examples/CyberiadaFormat-Autoborder.graphml',
                     id='parse ArduinoUno')
    ]
)
def test_parse(path: str):
    """Test CGML parsing."""
    parser = CGMLParser()
    with open(path, 'r') as f:
        parser.parseCGML(f.read())


@pytest.mark.parametrize(
    'path',
    [
        pytest.param('compiler/platforms/Arduino.json',
                     id='create ArduinoUno platform'),
    ]
)
def test_new_platform_creation(path: str):
    with open(path, 'r') as f:
        platform = Platform(**json.loads(f.read()))
    print(platform)


@pytest.mark.parametrize(
    'raw_trigger, expected',
    [
        pytest.param(
            """entry/
                LED1.on();
                timer1.start(1000);
                """,
            [
                InnerEvent(
                    InnerTrigger(
                        'entry',
                        None
                    ),
                    """
                LED1.on();
                timer1.start(1000);
                """
                )
            ]
        ),
        pytest.param(
            'timer1.timeout/',
            [
                InnerEvent(
                    InnerTrigger(
                        'timer1_timeout',
                        None
                    ),
                    ''
                )
            ]

        )
    ]
)
def test_parse_actions(raw_trigger: str, expected: str):
    assert __parse_actions(raw_trigger) == expected


@pytest.mark.asyncio
async def test_generating_code(init_platform):
    with open('examples/CyberiadaFormat-Blinker.graphml', 'r') as f:
        data = f.read()
        path = './test/test_folder/'
        with create_test_folder(path, 10):
            try:
                sm = await parse(data)
                await CppFileWriter(sm, True).write_to_file(path, 'ino')
                print('Code generated!')
            except Exception as e:
                print(e)


@pytest.mark.asyncio
async def test_cgml_route(init_platform):
    await AsyncPath(BUILD_DIRECTORY).mkdir(exist_ok=True)
    test_path = os.path.dirname(os.path.abspath(inspect.stack()[0][1]))
    with open('examples/CyberiadaFormat-Blinker.graphml', 'r') as f:
        path = test_path + '/test_project/sketch/'
        with create_test_folder(path, 2):
            data = f.read()
            result = await compile_xml(data, path)
            await create_response(path, result)
