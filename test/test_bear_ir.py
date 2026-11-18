"""CyberBear IR: notebook parity, wire format, streaming and real ARM builds."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
from xml.etree import ElementTree

import pytest

from compiler.CGML import parse
from compiler.fullgraphmlparser.graphml_to_cpp import CppFileWriter
from compiler.platform_manager import PlatformManager
from compiler.types.platform_types import Platform


ROOT = Path(__file__).resolve().parents[1]
FIXTURES = ROOT / 'test/bear_ir'
SOURCES = ROOT / 'compiler/library/blg-mb-1-a12/1.0/source'
PLATFORM_PATH = ROOT / 'compiler/platforms/blg-mb-1-a12/1.0/blg-mb-1-a12-1.0.json'
TEST_SCHEME = '''<?xml version="1.0" encoding="UTF-8"?>
<graphml xmlns="http://graphml.graphdrawing.org/xmlns">
<key id="gFormat" for="graphml" attr.name="format" attr.type="string"/>
<key id="dName" for="node" attr.name="name" attr.type="string"/>
<key id="dVertex" for="node" attr.name="vertex" attr.type="string"/>
<key id="dStateMachine" for="graph" attr.name="stateMachine" attr.type="string"/>
<key id="dNote" for="node" attr.name="note" attr.type="string"/>
<key id="dData" for="node" attr.name="data" attr.type="string"/>
<key id="dGeometry" for="node" attr.name="geometry"/>
<data key="gFormat">Cyberiada-GraphML-1.0</data>
<graph id="Infrared" edgedefault="directed">
<data key="dStateMachine"/>
<node id="meta"><data key="dNote">formal</data><data key="dName">CGML_META</data>
<data key="dData">platform/ blg-mb-1-a12

standardVersion/ 1.0

platformVersion/ 1.0

name/ ИК-приём и передача</data></node>
<node id="component_ir"><data key="dNote">formal</data><data key="dName">CGML_COMPONENT</data>
<data key="dData">id/ ir

type/ IR</data></node>
<node id="component_nose"><data key="dNote">formal</data><data key="dName">CGML_COMPONENT</data>
<data key="dData">id/ nose

type/ NoseBytes</data></node>
<node id="initial"><data key="dVertex">initial</data><data key="dGeometry"><point x="0" y="0"/></data></node>
<node id="exchange"><data key="dName">Обмен</data><data key="dData">entry/
ir.sendByte(150);

nose.isByteReceived/
ir.sendByte(nose.value);</data><data key="dGeometry"><rect x="100" y="100" width="300" height="160"/></data></node>
<edge id="start" source="initial" target="exchange"/>
</graph>
</graphml>
'''


@pytest.fixture(scope='module')
def native_modem(tmp_path_factory):
    """Build the production IR implementation with hardware stubs and UBSan."""
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ is needed for native IR tests')
    binary = tmp_path_factory.mktemp('bear-ir') / 'modem'
    subprocess.run([
        compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror',
        '-fsanitize=undefined', '-fno-sanitize-recover=all', f'-I{SOURCES}',
        str(FIXTURES / 'modem.cpp'), '-o', str(binary)
    ], check=True, capture_output=True, text=True)
    return binary


def test_streaming_protocol(native_modem):
    """All 256 bytes, phases, noise, queues, idle modes and incomplete frames."""
    subprocess.run([str(native_modem)], check=True, capture_output=True, text=True)


def test_original_notebook_recording(native_modem):
    """Compare every decimated envelope with original Python init()/next()."""
    reference = json.loads((FIXTURES / 'reference.json').read_text())
    raw = (FIXTURES / 'recording.txt').read_bytes()
    assert hashlib.sha256(raw).hexdigest() == reference['recording_sha256']
    # The notebook consumes one sample before its processing loop.
    samples = raw.decode().splitlines()[1:]
    result = subprocess.run([str(native_modem), 'trace'],
                            input='\n'.join(samples), check=True,
                            capture_output=True, text=True)
    envelopes, received = [], []
    for line in result.stdout.splitlines():
        parts = line.split()
        if parts[0] == 'byte':
            received.append(int(parts[1]))
        else:
            envelopes.append(list(map(float, parts[1:])))
    assert len(envelopes) == len(reference['envelope'])
    for actual, expected in zip(envelopes, reference['envelope']):
        assert actual == pytest.approx(expected, rel=0.0001, abs=0.00001)
    assert reference['bits'] == [1, 0, 1, 0, 1, 0, 1, 0]
    assert received == [0xaa]


def test_irpcb_transmitter(tmp_path):
    """A12 carrier/timer driver, standard IRPCB RX and mixed protocol FIFO."""
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ is needed for native IR tests')
    shutil.copyfile(ROOT / 'compiler/library/IRPCB/1.0/source/lib/ir.cpp',
                    tmp_path / 'ir.cpp')
    binary = tmp_path / 'a12-irpcb'
    subprocess.run([
        compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror',
        '-Wno-unused-parameter', '-fsanitize=undefined',
        '-fno-sanitize-recover=all', f'-I{tmp_path}',
        f'-I{ROOT / "test/irpcb/stubs"}',
        f'-I{ROOT / "compiler/library/IRPCB/1.0/source/lib"}',
        f'-I{SOURCES}', '-DIR_RX_ENABLED=1',
        str(FIXTURES / 'irpcb.cpp'), '-o', str(binary)
    ], check=True, capture_output=True, text=True)
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)


@pytest.mark.asyncio
@pytest.mark.parametrize('components', [
    ['IR'], ['NoseBytes'], ['EarsBytes'], ['IR', 'NoseBytes'],
    ['NoseBytes', 'IR', 'EarsBytes'], ['IRPcb'], ['IR', 'IRPcb'],
    ['IRPcb', 'IR'], ['NoseBytes', 'IRPcb', 'IR', 'EarsBytes'],
    ['NoseBytes', 'IR', 'EarsBytes', 'MatrixPicture']
])
async def test_generate_and_build(components, tmp_path):
    """Existing component interfaces and all modem include orders build for ARM."""
    platform = Platform(**json.loads(PLATFORM_PATH.read_text()))
    manager = PlatformManager()
    if not manager.is_loaded(platform.id, '1.0'):
        await manager.load_platform(str(PLATFORM_PATH))
    namespace = '{http://graphml.graphdrawing.org/xmlns}'
    ElementTree.register_namespace('', namespace[1:-1])
    document = ElementTree.fromstring(TEST_SCHEME)
    graph = document.find(f'{namespace}graph')
    for node in list(graph.findall(f'{namespace}node')):
        if node.attrib['id'].startswith('component_'):
            graph.remove(node)
    ids = {'IR': 'ir', 'IRPcb': 'irpcb', 'NoseBytes': 'nose', 'EarsBytes': 'ears',
           'MatrixPicture': 'matrix'}
    for component in components:
        node = ElementTree.SubElement(graph, f'{namespace}node', id=f'component_{component}')
        ElementTree.SubElement(node, f'{namespace}data', key='dNote').text = 'formal'
        ElementTree.SubElement(node, f'{namespace}data', key='dName').text = 'CGML_COMPONENT'
        ElementTree.SubElement(node, f'{namespace}data', key='dData').text = (
            f'id/ {ids[component]}\n\ntype/ {component}')
    body = 'entry/\n'
    if 'MatrixPicture' in components:
        body += 'matrix.draw(heart);\n'
    if 'IRPcb' in components:
        body += 'irpcb.sendByte(85);\n'
    if 'IR' in components:
        body += 'ir.sendByte(150);\n'
    if 'IRPcb' in components:
        body += 'irpcb.sendWord(4660);\n'
    for component in ['NoseBytes', 'EarsBytes']:
        if component in components:
            body += f'\n{ids[component]}.isByteReceived/\n'
            body += 'ir.sendByte(nose.value);\n' if component == 'NoseBytes' and 'IR' in components else ''
    graph.find(f'{namespace}node[@id="exchange"]/{namespace}data[@key="dData"]').text = body
    errors, machines = await parse(ElementTree.tostring(document, encoding='unicode'))
    assert not errors
    machine = machines['Infrared']
    await CppFileWriter(machine, True, True).write_to_file(str(tmp_path), 'cpp')
    for name in machine.compiling_settings.build_files:
        target = tmp_path / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(SOURCES / name, target)
    for name in ['qhsm.hpp', 'qhsm.cpp']:
        shutil.copyfile(ROOT / 'compiler/library/default/1.0/source' / name, tmp_path / name)
    (tmp_path / 'build').mkdir()
    if shutil.which('arm-none-eabi-g++') is None:
        return  # Code generation is still checked without an ARM toolchain.
    for command in platform.compiling_settings:
        result = subprocess.run([command.command, *command.flags], cwd=tmp_path,
                                capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr
    assert 0 < (tmp_path / 'build/sketch.bin').stat().st_size < 88 * 1024
