"""Rebuild the recording and golden envelopes from the original bear_opt.zip."""
import argparse
import ast
import hashlib
import json
import math
from pathlib import Path
import zipfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('archive', type=Path)
args = parser.parse_args()
output = Path(__file__).resolve().parent
z = zipfile.ZipFile(args.archive)
notebook = z.read('signal_reciever.ipynb')
source = '\n\n'.join(''.join(c['source']) for c in json.loads(notebook)['cells'] if c['cell_type'] == 'code')
tree = ast.parse(source)
functions = [n for n in tree.body if isinstance(n, (ast.FunctionDef, ast.ClassDef)) and n.name in ('init', 'Get_Sample', 'next')]
ns = dict(period_0=4, period_1=32, alpha=1/48, alpha_m_1=47/48,
          a_0_hpf=(1+math.exp(-1/64))/2, a_1_hpf=-(1+math.exp(-1/64))/2,
          b_1_hpf=math.exp(-1/64), x_1_hpf=0, y_1_hpf=0)
raw = z.read('блок_2_13050.txt')
(output / 'recording.txt').write_bytes(raw)
ns['signal'] = list(map(float, raw.split()))
exec(compile(ast.Module(body=functions, type_ignores=[]), '<original-notebook>', 'exec'), ns)
assignment = next(n for n in tree.body if isinstance(n, ast.Assign) and isinstance(n.targets[0], ast.Tuple) and isinstance(n.value, ast.Call) and isinstance(n.value.func, ast.Name) and n.value.func.id == 'init')
exec(compile(ast.Module(body=[assignment], type_ignores=[]), '<original-init>', 'exec'), ns)
ns['get_sample'] = ns['Get_Sample'](ns['signal'])
ns['get_sample'].get()  # Notebook consumes the first sample before its loop.
trace = []
f0 = f1 = 0
for i in range(len(ns['signal'])-1):
    ns['next']()
    if i % 16 == 0:
        f0 = (f0 + ns['ff_0']) / 2
        f1 = (f1 + ns['ff_1']) / 2
        trace.append([f0, f1])
# Original center/peak detector and bit comparison, run once as in the notebook.
h = None
bits = []
for i in range(70, len(trace)):
    c = i - 31
    def energy(offset):
        return trace[c+offset][0] if c+offset >= 0 else 0
    if h is None and energy(0) > 40 and energy(0)/2 > energy(-15) and energy(0)/2 > energy(15) and energy(0) >= energy(-1) and energy(0) >= energy(1):
        h = i
    if h is not None and i-h in range(32,145,16):
        bits.append(int(trace[c][0] <= trace[c][1]))
assert bits == [1,0,1,0,1,0,1,0], bits
out = dict(notebook='signal_reciever.ipynb', notebook_sha256=hashlib.sha256(notebook).hexdigest(),
           recording='блок_2_13050.txt', recording_sha256=hashlib.sha256(raw).hexdigest(),
           note='Original init() and next(), no scaling or added noise; skip first input sample as in notebook. HPF tau=64, alpha=1/48, decimation=16, alpha_dec=1/2.',
           bits=bits, header_decimated_index=h, envelope=trace)
(output / 'reference.json').write_text(json.dumps(out, ensure_ascii=False, separators=(',', ':'))+'\n')
print('Original notebook:',len(trace),'envelopes, header',h,'bits',bits)
