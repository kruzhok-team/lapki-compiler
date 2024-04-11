import json
from compiler.types.platform_types import Platform


def test_not_compile_platform_creation():
    with open('compiler/platforms/Autoborder-new.json', 'r') as f:
        data = json.load(f)
    Platform(**data)
