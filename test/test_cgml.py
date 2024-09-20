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
from compiler.config import get_config
from compiler.handler import compile_xml, create_response
from cyberiadaml_py.cyberiadaml_parser import CGMLParser
from compiler.fullgraphmlparser.graphml_to_cpp import CppFileWriter
from compiler.types.platform_types import Platform
from compiler.CGML import parse
from compiler.platform_manager import PlatformManager

pytest_plugins = ('pytest_asyncio',)


async def init_platform():
    """Init ArduinoUno platform."""
    platform_manager = PlatformManager()
    if not platform_manager.platform_exist('ArduinoUno'):
        await platform_manager.load_platform('compiler/platforms/ArduinoUno/'
                                             '1.0/ArduinoUno-1.0.json')
    if not platform_manager.platform_exist('tjc-ms1-main'):
        await platform_manager.load_platform('compiler/platforms/tjc-ms1-main/'
                                             '1.0/tjc-ms1-main-1.0.json')
    if not platform_manager.platform_exist('tjc-ms1-mtrx-a1'):
        await platform_manager.load_platform('compiler/platforms/'
                                             'tjc-ms1-mtrx-a1/'
                                             '1.0/tjc-ms1-mtrx-a1-1.0.json')


@contextmanager
def create_test_folder(path: str, wait_time: int):
    """Create test folder by path and delete it\
        after exit from 'with' statement and wait time."""
    try:
        Path(path).mkdir(parents=True)
        yield
    finally:
        time.sleep(wait_time)
        shutil.rmtree(path)


@pytest.mark.parametrize(
    'path',
    [
        pytest.param('examples/CyberiadaFormat-Blinker.graphml',
                     id='parse ArduinoUno')
    ]
)
def test_parse(path: str):
    """Test CGML parsing."""
    parser = CGMLParser()
    with open(path, 'r') as f:
        parser.parse_cgml(f.read())


@pytest.mark.parametrize(
    'path',
    [
        pytest.param('compiler/platforms/ArduinoUno/1.0/ArduinoUno-1.0.json'),
    ]
)
def test_new_platform_creation(path: str):
    """Test Platform object creation."""
    with open(path, 'r') as f:
        Platform(**json.loads(f.read()))


@pytest.mark.asyncio
async def test_generating_code():
    """Test generating code without compiling."""
    await init_platform()
    with open('examples/CyberiadaFormat-Blinker.graphml', 'r') as f:
        data = f.read()
        path = './test/test_folder/'
        with create_test_folder(path, 0):
            try:
                state_machines = await parse(data)
                for sm in state_machines.values():
                    await CppFileWriter(sm, True).write_to_file(path, 'ino')
                print('Code generated!')
            except Exception as e:
                print(e)


@pytest.mark.parametrize('scheme_path, platform_id', [
    pytest.param(
        'examples/CyberiadaFormat-Blinker.graphml',
        'ArduinoUno'
    ),
    pytest.param(
        'examples/ms1-mtrx.graphml',
        'tjc-ms1-mtrx-a1'
    ),
    pytest.param(
        'examples/ms1-main.graphml',
        'tjc-ms1-main'
    ),
    pytest.param(
        'examples/ms1-btn.graphml',
        'tjc-ms1-btn-a2'
    ),
    pytest.param(
        'examples/ms1-lmp.graphml',
        'tjc-ms1-lmp-a3'
    ),
    pytest.param(
        'examples/choices.graphml',
        'ArduinoUno'
    ),
    pytest.param(
        'examples/with-final.graphml',
        'ArduinoUno'
    ),
    pytest.param(
        'examples/two_choices.graphml',
        'ArduinoUno'
    ),
    pytest.param(
        'examples/initial_states.graphml',
        'ArduinoUno'
    ),
    pytest.param(
        'examples/with-defer.xml',
        'ArduinoUno'
    ),
    pytest.param(
        'examples/with-propagate-block.graphml',
        'ArduinoUno'
    ),
])
@pytest.mark.asyncio
async def test_compile_schemes(scheme_path: str, platform_id: str):
    """Testing compiling and code generation from CGML-schemes."""
    # TODO: Добавить платформу в качестве аргумента теста
    await AsyncPath(get_config().build_directory).mkdir(exist_ok=True)
    platform_manager = PlatformManager()
    test_path = os.path.dirname(os.path.abspath(inspect.stack()[0][1]))
    await init_platform()
    with open(scheme_path, 'r') as f:
        path = test_path + '/test_project/sketch/'
        with create_test_folder(path, 0):
            data = f.read()
            print(path)
            result = await compile_xml(data, path)
            filecount = 0
            for sm in result.keys():
                print(result[sm][0])
                await create_response(path, result)
                dir = AsyncPath(path + sm + '/sketch/' + 'build/')
                filecount = len([file async for file in dir.iterdir()])
                assert filecount != 0
            # Когда мы запускаем все тесты сразу, PlatformManager не очищается,
            # поэтому нужно удалять версии вручную
            versions = platform_manager._delete_from_version_registry(
                platform_id, set(['1.0']))
            platforms = (
                platform_manager._delete_versions_from_platform_registry(
                    platform_id, set(['1.0']))
            )
            platform_manager.platforms = platforms
            platform_manager.platforms_info = versions
            assert filecount != 0
