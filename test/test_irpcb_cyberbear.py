"""IRPCB exchange: shared RAM, legacy formats and A12 interoperability."""
import json
from pathlib import Path
import re
import shutil
import subprocess
from xml.etree import ElementTree

import pytest

from compiler.CGML import parse
from compiler.fullgraphmlparser.graphml_to_cpp import CppFileWriter
from compiler.platform_manager import PlatformManager
from compiler.types.platform_types import Platform


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'compiler/library/IRPCB/1.0/source'
FIXTURES = ROOT / 'test/irpcb'
PLATFORM_PATH = ROOT / 'compiler/platforms/IRPCB/1.0/IRPCB-1.0.json'


def test_native_transmitter(tmp_path):
    """Check both formats, DMA ownership and 256 bytes with real A12 RX."""
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ is needed for the native transmitter check')
    shutil.copyfile(SOURCE / 'lib/ir.cpp', tmp_path / 'ir.cpp')
    binary = tmp_path / 'transmitter'
    subprocess.run([
        compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror',
        '-Wno-unused-parameter', '-fsanitize=undefined',
        '-fno-sanitize-recover=all', f'-I{tmp_path}',
        f'-I{FIXTURES / "stubs"}',
        f'-I{ROOT / "compiler/library/blg-mb-1-a12/1.0/source"}',
        str(FIXTURES / 'transmitter.cpp'), '-o', str(binary)
    ], check=True, capture_output=True, text=True)
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)

@pytest.mark.parametrize('driver', [False, True])
def test_native_receiver(tmp_path, driver):
    """Decode the real A12 stream and exercise the timer ISR and mode switch."""
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ is needed for receiver tests')
    binary = tmp_path / 'receiver'
    flags = ['-std=c++17', '-Wall', '-Wextra', '-Werror',
             '-Wno-unused-parameter', '-fsanitize=undefined',
             '-fno-sanitize-recover=all', f'-I{SOURCE / "lib"}']
    if driver:
        shutil.copyfile(SOURCE / 'lib/ir.cpp', tmp_path / 'ir.cpp')
        # Use the real board definition; the simulated wire is fixed to the
        # hardware /IR_RCVR net (PA1) independently of that definition.
        pin = re.search(r'^#define\s+IR_RX_DATA_PIN\s+PA(\d+)\b',
                        (SOURCE / 'board.h').read_text(), re.MULTILINE)
        assert pin is not None
        flags = [f'-I{tmp_path}', f'-I{FIXTURES / "stubs"}',
                 '-DIR_RX_ENABLED=1', f'-DIR_RX_DATA_PIN={pin[1]}', *flags]
    else:
        flags.append(
            f'-I{ROOT / "compiler/library/blg-mb-1-a12/1.0/source"}')
    name = 'receiver_driver.cpp' if driver else 'receiver.cpp'
    subprocess.run([compiler, *flags, str(FIXTURES / name), '-o', str(binary)],
                   check=True, capture_output=True, text=True)
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)


def test_dma_channel_flag_clear(tmp_path):
    """Clear the requested channel flags without touching its peers."""
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ is needed for the DMA register check')
    source = (SOURCE / 'lib/gd_lib.cpp').read_text()
    # Compile the actual method with host registers; the rest is MCU-specific.
    method = source[source.index('void DMA_t::ClearIrq()'):]
    method = method[:method.index('\nvoid DMA_t::DisableAndClearIRQ')]
    program = '''
#include <cassert>
#include <cstdint>
struct Registers { uint32_t INTC; } a, b;
Registers *DMA0 = &a, *DMA1 = &b;
class DMA_t {
public:
    unsigned chnl_n;
    void ClearIrq() const;
};
''' + method + '''
int main() {
    for (unsigned ch = 0; ch < 12; ++ch) {
        a.INTC = b.INTC = 0;
        DMA_t dma{ch};
        dma.ClearIrq();
        assert(a.INTC == (ch < 7 ? 15u << (ch * 4) : 0));
        assert(b.INTC == (ch >= 7 ? 15u << ((ch - 7) * 4) : 0));
    }
}
'''
    cpp, binary = tmp_path / 'dma.cpp', tmp_path / 'dma'
    cpp.write_text(program)
    subprocess.run([compiler, '-std=c++17', str(cpp), '-o', str(binary)],
                   check=True, capture_output=True, text=True)
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)


@pytest.fixture(scope='module')
def artifacts(tmp_path_factory):
    """Share the library object cache within each build configuration."""
    return tmp_path_factory.mktemp('irpcb-artifacts')


@pytest.mark.asyncio
@pytest.mark.parametrize('configuration', ['Fromboot', 'Debug'])
@pytest.mark.parametrize('transmitter', [
    'none', 'IRTransmitter', 'BlgIrTransmitter',
    'IRReciever', 'BlgIrReceiver', 'duplex'
])
async def test_generate_build_and_ram(
        configuration, transmitter, tmp_path, artifacts):
    """Build all modes and duplex schemes with at least 3 KiB heap."""
    platform = Platform(**json.loads(PLATFORM_PATH.read_text()))
    manager = PlatformManager()
    if not manager.is_loaded(platform.id, '1.0'):
        await manager.load_platform(str(PLATFORM_PATH))
    namespace = '{http://graphml.graphdrawing.org/xmlns}'
    ElementTree.register_namespace('', namespace[1:-1])
    document = ElementTree.fromstring(
        (ROOT / 'examples/IRPCB_LED_EXAMPLE.graphml').read_text())
    graph = document.find(f'{namespace}graph')
    components = [] if transmitter == 'none' else [transmitter]
    if transmitter == 'duplex':
        components = ['BlgIrTransmitter', 'BlgIrReceiver']
    state_id = 'ztcldydpeocjcqgglcab'
    for component in components:
        node = ElementTree.SubElement(
            graph, f'{namespace}node', id=f'component_{component}')
        ElementTree.SubElement(
            node, f'{namespace}data', key='dNote').text = 'formal'
        ElementTree.SubElement(
            node, f'{namespace}data', key='dName').text = 'CGML_COMPONENT'
        ElementTree.SubElement(node, f'{namespace}data', key='dData').text = (
            f'id/ {component}\n\ntype/ {component}')
        data = graph.find(f'{namespace}node[@id="{state_id}"]/'
                          f'{namespace}data[@key="dData"]')
        if component.endswith('Transmitter'):
            action = f'{component}::setPower(90);\n'
            action += f'{component}::transmit(150);'
        else:
            action = f'{component}::printPkg();'
            edge = ElementTree.SubElement(
                graph, f'{namespace}edge', id=f'event_{component}',
                source=state_id, target=state_id)
            ElementTree.SubElement(
                edge, f'{namespace}data', key='dData').text = (
                    f'{component}.isUpdated/\n{component}::printPkg();')
            if component == 'BlgIrReceiver':
                action += '\nBlgIrReceiver::printDiagnostics();'
        data.text = data.text.replace(
            'Led1.fadeIn(1500);', 'Led1.fadeIn(1500);\n' + action)
    xml = ElementTree.tostring(document, encoding='unicode')
    errors, machines = await parse(xml)
    assert not errors
    machine = machines['Machine1']
    await CppFileWriter(machine, True, True).write_to_file(
        str(tmp_path), 'cpp')
    generated = (tmp_path / 'sketch.cpp').read_text()
    for component in components:
        assert f'{component}::init();' in generated
        method = ('transmit(150)' if component.endswith('Transmitter')
                  else 'isUpdated()')
        assert f'{component}::{method}' in generated
    for name in ['qhsm.hpp', 'qhsm.cpp']:
        shutil.copyfile(ROOT / 'compiler/library/default/1.0/source' / name,
                        tmp_path / name)
    make, compiler = shutil.which('make'), shutil.which('arm-none-eabi-g++')
    if make is None or compiler is None:
        return  # Generation still runs on hosts without the ARM toolchain.
    build = tmp_path / 'build'
    build.mkdir()
    result = subprocess.run([
        make, '-C', str(SOURCE), f'build_{configuration}',
        f'EXTRA_DIR={tmp_path}', f'BUILD_DIR={build}',
        f'OUT_DIR={artifacts / configuration}',
        f'BUILD_HEX={build / "IR_PCB.hex"}'
    ], capture_output=True, text=True, timeout=180)
    assert result.returncode == 0, result.stdout + result.stderr
    elf = build / 'IR_PCB.elf'
    sections = subprocess.run(
        ['arm-none-eabi-size', '-A', str(elf)], check=True,
        capture_output=True, text=True).stdout
    sizes = {line.split()[0]: int(line.split()[1])
             for line in sections.splitlines() if line.startswith('.')}
    assert sizes['.heap'] >= 3 * 1024, sections
    symbols = subprocess.run(['arm-none-eabi-nm', '-S', str(elf)],
                             check=True, capture_output=True, text=True).stdout
    dac = [line for line in symbols.splitlines() if 'IRLed7dac_buf' in line]
    assert len(dac) == 1 and int(dac[0].split()[1], 16) == 3996, dac
    print(f'{configuration}/{transmitter}: heap={sizes[".heap"]}, '
          f'bss={sizes[".bss"]}, DAC=3996 bytes')
