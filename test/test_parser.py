"""Module implements testing yed-Graphml parsing."""
import json

import pytest
from compiler.GraphmlParser import GraphmlParser
from compiler.Logger import Logger
from compiler.PlatformManager import PlatformManager
from compiler.config import get_config

pytest_plugins = ('pytest_asyncio',)


@pytest.mark.asyncio
@pytest.mark.parametrize('path, platform', [
    pytest.param('examples/old/bearlogaSchemas/Autoborder_'
                 '638330223036439120.graphml',
                 'BearlogaDefend-Autoborder', id='Autoborder'),
    pytest.param('examples/old/bearlogaSchemas/Generator_'
                 '638331191524332730.graphml',
                 'BearlogaDefend-Generator', id='Generator'),
    pytest.param('examples/old/bearlogaSchemas/Smoker_'
                 '638331191988353340.graphml',
                 'BearlogaDefend-Smoker', id='Smoker'),
    pytest.param('examples/old/bearlogaSchemas/Stapler_'
                 '638331190677085090.graphml',
                 'BearlogaDefend-Stapler', id='Stapler'),
])
async def test_graphmlParser(path: str, platform: str):
    """Test parsing yed-Graphml."""
    platform_manager = PlatformManager()
    await Logger.init_logger()
    await platform_manager.init_platforms(get_config().platform_directory)
    with open(path, 'r') as f:
        unprocessed_xml = f.read()

    parsed_graphml = await GraphmlParser.parse(
        unprocessed_xml=unprocessed_xml, platform=platform)

    assert parsed_graphml is not None

    with open(f'examples/old/bearlogaSchemas/{platform}.json', 'w') as f:
        json.dump(parsed_graphml, f, ensure_ascii=False, indent=3)
    assert parsed_graphml.get('states') is not None
