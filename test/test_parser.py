import pytest
import json


from compiler.CJsonParser import CJsonParser, Labels
from compiler.SourceFile import SourceFile
from compiler.GraphmlParser import GraphmlParser
from compiler.Logger import Logger
from compiler.PlatformManager import PlatformManager
from compiler.config import SCHEMA_DIRECTORY

pytest_plugins = ('pytest_asyncio',)


@pytest.mark.asyncio
async def test_getfiles():
    with open('examples/ExampleRequestSource.json', 'r') as f:
        data = json.load(f)

    files = await CJsonParser.getFiles(data["source"])
    assert files == [SourceFile("cpp_example", "cpp", "*code here*"),
                     SourceFile("cpp_example", "hpp", "*code here*")]


@pytest.mark.parametrize("variables, expected", [
    pytest.param({
        "myVariable": {
            "type": "int",
            "value": "1000"
        }
    }, ("\nstatic int myVariable;", "\nint User::myVariable = 1000;"))
])
def test_getUserVariable(variables, expected):
    result = CJsonParser.getUserVariables(variables)
    assert result == expected


@pytest.mark.asyncio
@pytest.mark.parametrize("path, platform", [
    pytest.param("examples/bearlogaSchemas/Autoborder_638330223036439120.graphml",
                 "BearlogaDefend-Autoborder", id="Autoborder"),
    pytest.param("examples/bearlogaSchemas/Generator_638331191524332730.graphml",
                 "BearlogaDefend-Generator", id="Generator"),
    pytest.param("examples/bearlogaSchemas/Smoker_638331191988353340.graphml",
                 "BearlogaDefend-Smoker", id="Smoker"),
    pytest.param("examples/bearlogaSchemas/Stapler_638331190677085090.graphml",
                 "BearlogaDefend-Stapler", id="Stapler"),
])
async def test_graphhmlParser(path: str, platform: str):
    await Logger.init_logger()
    await PlatformManager.initPlatform(SCHEMA_DIRECTORY)
    with open(path, 'r') as f:
        unprocessed_xml = f.read()

    parsed_graphml = await GraphmlParser.parse(
        unprocessed_xml=unprocessed_xml, platform=platform)

    with open(f"examples/bearlogaSchemas/{platform}.json", "w") as f:
        json.dump(parsed_graphml, f, ensure_ascii=False, indent=3)

    assert parsed_graphml["states"] is not None
