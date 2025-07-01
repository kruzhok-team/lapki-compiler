"""Module with windows commands."""
from typing import Set
import asyncio


async def nt_copy(
    path_to_libs: Set[str],
    target_directory: str,
    cwd: str
) -> None:
    """Copy files from path_to_libs to target directory."""
    for lib in path_to_libs:
        await asyncio.create_subprocess_shell(f'copy "{lib}" '
                                              f'"{target_directory}"',
                                              cwd=cwd,
                                              stdout=asyncio.subprocess.PIPE)


def nt_decode(
    data: bytes,
) -> str:
    """Decode bytes as cp1251."""
    return data.decode('cp1251')
