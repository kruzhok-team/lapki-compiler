"""Atmospheric boards: driver behavior, generation and optional ARM builds."""
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


def sources(revision):
    """Return the self-contained library for a board revision."""
    return ROOT / 'compiler/library' / f'tjc-ms2-atm-{revision}/1.0/source'


def load_platform(revision):
    """Read and validate a platform using the production schema."""
    platform_id = f'tjc-ms2-atm-{revision}'
    path = ROOT / 'compiler/platforms' / platform_id / '1.0'
    schema = path / f'{platform_id}-1.0.json'
    return Platform(**json.loads(schema.read_text()))


def test_revision_components():
    """A2 cannot offer absent hardware; S4/S5 remain one component type."""
    a1, a2 = load_platform('a1'), load_platform('a2')
    assert set(a1.components) == {
        'SimpleBus', 'DataBus', 'RGBLed', 'SCD41', 'HP203B',
        'XGZP6859D', 'SHT30',
    }
    assert set(a2.components) == set(a1.components) - {'XGZP6859D'}
    a1_position = a1.components['SHT30'].constructorParameters['position']
    a2_position = a2.components['SHT30'].constructorParameters['position']
    assert a1_position.type == ['4', '5']
    assert a2_position.type == ['5']


def test_packaged_sources():
    """Keep shared drivers identical in both exported libraries."""
    for path in sources('a1').iterdir():
        other = sources('a2') / path.name
        assert other.is_file()
        if path.name != 'Board.hpp':
            assert path.read_bytes() == other.read_bytes(), path.name
    assert 'scd41Bus = 1' in (sources('a1') / 'Board.hpp').read_text()
    assert 'scd41Bus = 2' in (sources('a2') / 'Board.hpp').read_text()
    for revision in ['a1', 'a2']:
        platform = load_platform(revision)
        required = set(platform.default_build_files)
        for component in platform.components.values():
            required.update(component.buildFiles)
        assert all((sources(revision) / name).is_file() for name in required)


def test_native_sensor_protocols(tmp_path):
    """Check CRC, units, atomic samples, recovery, readiness and wraparound."""
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ is needed for the native driver tests')
    executable = tmp_path / 'sensors'
    subprocess.run([
        compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror',
        '-fsanitize=undefined', f'-I{sources("a1")}',
        str(ROOT / 'test/ms2_atm/sensors.cpp'), '-o', str(executable)
    ], check=True, capture_output=True, text=True)
    subprocess.run(
        [str(executable)], check=True, capture_output=True, text=True
    )


@pytest.mark.asyncio
@pytest.mark.parametrize('revision', ['a1', 'a2'])
@pytest.mark.parametrize('bus', ['SimpleBus', 'DataBus', 'none'])
async def test_generate_and_build(revision, bus, tmp_path, monkeypatch):
    """Generate real GraphML examples and build their declared file sets."""
    platform = load_platform(revision)
    monkeypatch.setattr(PlatformManager, '_instance', None)
    manager = PlatformManager()
    platform_path = ROOT / 'compiler/platforms' / platform.id / '1.0'
    await manager.load_platform(str(platform_path / f'{platform.id}-1.0.json'))
    xml = (ROOT / f'examples/ms2-atm-{revision}.graphml').read_text()
    if bus == 'DataBus':
        xml = xml.replace('type/ SimpleBus', 'type/ DataBus')
        xml = xml.replace('bus.setAddress(1);\n', '')
        xml = xml.replace('bus.packetReceived', 'bus.isByteReceived')
        xml = xml.replace(
            'bus.sendPacket(2, bus.lastData);', 'bus.sendByte(bus.lastByte);'
        )
    elif bus == 'none':
        # No component may require an unrelated UART or sensor implementation.
        namespace = '{http://graphml.graphdrawing.org/xmlns}'
        ElementTree.register_namespace('', namespace[1:-1])
        document = ElementTree.fromstring(xml)
        graph = document.find(f'{namespace}graph')
        assert graph is not None
        for node in list(graph.findall(f'{namespace}node')):
            if node.attrib['id'].startswith('component_'):
                graph.remove(node)
            elif node.attrib['id'] == 'monitor':
                data = node.find(f'{namespace}data[@key="dData"]')
                assert data is not None
                data.text = 'entry/'
        xml = ElementTree.tostring(document, encoding='unicode')
    errors, machines = await parse(xml)
    assert not errors
    machine = machines['Atmosphere']
    writer = CppFileWriter(machine, True, True)
    await writer.write_to_file(str(tmp_path), 'cpp')
    generated = '\n'.join(
        path.read_text() for path in tmp_path.glob('*.?pp')
    )
    expected_sensors = []
    if bus != 'none':
        expected_sensors = ['s2', 's3', 's5']
        if revision == 'a1':
            expected_sensors.extend(['s4', 's6'])
    for component in expected_sensors:
        assert f'{component}.begin();' in generated
        assert f'{component}.poll();' in generated
        assert f'{component}.valueReceived()' in generated
        assert f'{component}.readingError()' in generated
    assert 'valid' not in generated
    settings = machine.compiling_settings
    assert settings is not None
    for name in settings.build_files:
        shutil.copyfile(sources(revision) / name, tmp_path / name)
    default_sources = ROOT / 'compiler/library/default/1.0/source'
    for name in ['qhsm.hpp', 'qhsm.cpp']:
        shutil.copyfile(default_sources / name, tmp_path / name)
    (tmp_path / 'build').mkdir()
    if not shutil.which('arm-none-eabi-g++'):
        return  # generation remains covered on hosts without the ARM toolchain
    compile_commands = platform.compiling_settings
    assert compile_commands is not None
    for command in compile_commands:
        result = subprocess.run(
            [command.command, *command.flags], cwd=tmp_path,
            capture_output=True, text=True
        )
        assert result.returncode == 0, result.stdout + result.stderr
    assert 0 < (tmp_path / 'build/sketch.bin').stat().st_size < 65536


@pytest.mark.parametrize('first,second', [
    ('SimpleBus', 'DataBus'), ('DataBus', 'SimpleBus')
])
def test_bus_conflict(first, second, tmp_path):
    """Reject two owners of the same UART in both include orders."""
    compiler = shutil.which('arm-none-eabi-g++')
    if compiler is None:
        pytest.skip('ARM compiler is needed for the UART ownership check')
    sketch = tmp_path / 'conflict.cpp'
    sketch.write_text(f'#include "{first}.hpp"\n#include "{second}.hpp"\n')
    result = subprocess.run([
        compiler, '-std=c++11', '-mcpu=cortex-m0plus', '-mthumb',
        f'-I{sources("a1")}', '-fsyntax-only', str(sketch)
    ], capture_output=True, text=True)
    assert result.returncode != 0
    assert 'SimpleBus and DataBus share USART1' in result.stderr
