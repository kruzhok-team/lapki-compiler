"""Module for testing CGML.py file."""
import json
import pytest
from cyberiadaml_py.cyberiadaml_parser import CGMLParser
from compiler.types.inner_types import InnerEvent, InnerTrigger
from compiler.types.platform_types import Platform
from compiler.CGML import __parse_actions


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
