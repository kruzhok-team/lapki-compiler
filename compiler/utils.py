"""Utility functions."""
import os.path
from datetime import datetime
from typing import List

from compiler.config import get_config


def get_filename(path: str) -> str:
    """Get path without suffixes."""
    print(path)
    return path.split('.')[0]


def get_file_extension(suffixes: List[str]) -> str:
    """Create string extension from Path.suffixes."""
    return ''.join(suffixes).replace('.', '', 1)


def get_project_directory() -> str:
    """Generate path to project directory, but don't create it."""
    base_dir = str(datetime.now()) + '/'
    base_dir = base_dir.replace(' ', '_')
    return os.path.join(get_config().module_directory, base_dir)
