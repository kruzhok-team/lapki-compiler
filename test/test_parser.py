"""Module implements testing yed-Graphml parsing."""
import json

import pytest
from compiler.graphml_parser import GraphmlParser
from compiler.logger import Logger
from compiler.platform_manager import PlatformManager

pytest_plugins = ('pytest_asyncio',)


@pytest.mark.asyncio
@pytest.mark.parametrize('path, path_to_platform, platform', [
    pytest.param('examples/old/bearlogaSchemas/Autoborder_'
                 '638330223036439120.graphml',
                 'compiler/platforms/BearlogaDefend-Autoborder/'
                 '1.0/BearlogaDefend-Autoborder-1.0.json',
                 'BearlogaDefend-Autoborder', id='Autoborder'),
    pytest.param('examples/old/bearlogaSchemas/Generator_'
                 '638331191524332730.graphml',
                 'compiler/platforms/BearlogaDefend-Generator/1.0/'
                 'BearlogaDefend-Generator-1.0.json',
                 'BearlogaDefend-Generator', id='Generator'),
    pytest.param('examples/old/bearlogaSchemas/Smoker_'
                 '638331191988353340.graphml',
                 'compiler/platforms/BearlogaDefend-Smoker/'
                 '1.0/BearlogaDefend-Smoker-1.0.json',
                 'BearlogaDefend-Smoker', id='Smoker'),
    pytest.param('examples/old/bearlogaSchemas/Stapler_'
                 '638331190677085090.graphml',
                 'compiler/platforms/BearlogaDefend-Stapler/'
                 '1.0/BearlogaDefend-Stapler-1.0.json',
                 'BearlogaDefend-Stapler', id='Stapler'),
])
async def test_graphmlParser(path: str, path_to_platform: str, platform: str):
    """Test parsing yed-Graphml."""
    platform_manager = PlatformManager()
    platform_obj = await platform_manager.load_platform(path_to_platform)
    await Logger.init_logger()
    with open(path, 'r') as f:
        unprocessed_xml = f.read()

    parsed_graphml = await GraphmlParser.parse(
        unprocessed_xml=unprocessed_xml, platform=platform)

    assert parsed_graphml is not None

    with open(f'examples/old/bearlogaSchemas/{platform}.json', 'w') as f:
        json.dump(parsed_graphml, f, ensure_ascii=False, indent=3)
    assert parsed_graphml.get('states') is not None
    versions_info = platform_manager._delete_from_version_registry(
        platform_obj.id, set([platform_obj.version]))
    platform_manager.platforms_info = versions_info
