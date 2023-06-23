import pytest
from compiler.JsonParser import JsonParser
from compiler.SourceFile import SourceFile
import json
pytest_plugins = ('pytest_asyncio',)

@pytest.mark.asyncio
async def test_getfiles():
    with open('src/test/Examples/ExampleRequestSource.json', 'r') as f:
        data = json.load(f)
    
    files = await JsonParser.getFiles(data)
    assert files == [SourceFile("cpp_example", "cpp", "*code here*"), SourceFile("cpp_example", "hpp", "*code here*")]