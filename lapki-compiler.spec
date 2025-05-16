# -*- mode: python ; coding: utf-8 -*-
import os

def collect_dir(src_root, prefix):
    for root, dirs, files in os.walk(src_root):
        for fname in files:
            src = os.path.join(root, fname)
            rel = os.path.relpath(root, 'compiler')
            dst = os.path.join('compiler', rel)
            yield (src, dst)

datas = []
datas += list(collect_dir('compiler/platforms', 'compiler/platforms'))
datas += list(collect_dir('compiler/library',   'compiler/library'))
datas.append(('compiler/ACCESS_TOKENS.txt', 'compiler'))


a = Analysis(
    ['compiler\\__main__.py'],
    pathex=[],
    binaries=[],
    datas=datas,
    hiddenimports=[],
    hookspath=[],     
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)

pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    [],
    exclude_binaries=False,
    name='lapki-compiler',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)

coll = COLLECT(
    exe,
    a.binaries,
    a.datas,
    strip=False,
    upx=True,
    upx_exclude=[],
    name='lapki-compiler',
)