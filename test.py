import asyncio
import json


try:
    from .JsonConverter import JsonConverter
except ImportError:
    from compiler.JsonConverter import JsonConverter
    from compiler.CJsonParser import CJsonParser

async def main():
    with open("compiler/schemas/berlogaScheme.json", 'r') as f:
        data = f.read()
    json_data = json.loads(data)
    sm = await CJsonParser.parseStateMachine(json_data, compiler="Berloga")
    converter = JsonConverter("a")
    await converter.parse(sm["states"])


asyncio.run(main())