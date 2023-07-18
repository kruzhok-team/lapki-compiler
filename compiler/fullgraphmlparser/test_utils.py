import os
import shutil
import stat

def is_boring_line(line: str) -> bool:
    line = line.strip()
    if not line:
        return True
    if line.startswith('/*'):
        return True
    return False

def remove_boring_lines(code: str) -> str:
    return '\n'.join([line for line in code.split('\n') if not is_boring_line(line)])

def removeOutputFolder():
    if (os.path.exists('testdata/test_output')):
        # Special magic is required to delete read-only files on Windows.
        # See https://bugs.python.org/issue19643 and https://hg.python.org/cpython/rev/31d63ea5dffa
        def remove_readonly(action, name, exc):
            os.chmod(name, stat.S_IWRITE)
            os.remove(name)

        shutil.rmtree('testdata/test_output', onerror=remove_readonly)
