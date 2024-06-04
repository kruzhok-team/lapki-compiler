"""Module implements testing yed-Graphml parsing"""
import json

import pytest
from compiler.GraphmlParser import GraphmlParser
from compiler.Logger import Logger
from compiler.PlatformManager import PlatformManager
from compiler.config import PLATFORM_DIRECTORY

pytest_plugins = ('pytest_asyncio',)


@pytest.mark.asyncio
@pytest.mark.parametrize('path, platform', [
    pytest.param('examples/bearlogaSchemas/Autoborder_638330223036439120.graphml',
                 'BearlogaDefend-Autoborder', id='Autoborder'),
    pytest.param('examples/bearlogaSchemas/Generator_638331191524332730.graphml',
                 'BearlogaDefend-Generator', id='Generator'),
    pytest.param('examples/bearlogaSchemas/Smoker_638331191988353340.graphml',
                 'BearlogaDefend-Smoker', id='Smoker'),
    pytest.param('examples/bearlogaSchemas/Stapler_638331190677085090.graphml',
                 'BearlogaDefend-Stapler', id='Stapler'),
])
async def test_graphhmlParser(path: str, platform: str):
    """Test parsing yed-Graphml."""
    platform_manager = PlatformManager()
    await Logger.init_logger()
    await platform_manager.init_platforms(PLATFORM_DIRECTORY)
    with open(path, 'r') as f:
        unprocessed_xml = f.read()

    parsed_graphml = await GraphmlParser.parse(
        unprocessed_xml=unprocessed_xml, platform=platform)

    assert parsed_graphml is not None

    with open(f'examples/bearlogaSchemas/{platform}.json', 'w') as f:
        json.dump(parsed_graphml, f, ensure_ascii=False, indent=3)
    assert parsed_graphml.get('states') is not None
