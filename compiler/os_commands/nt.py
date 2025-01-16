from typing import Set

import asyncio

async def nt_copy(path_to_libs: Set[str], target_directory: str, cwd: str) -> None:
    for lib in path_to_libs:
        await asyncio.create_subprocess_shell(f'copy "{lib}" "{target_directory}"', cwd=cwd, stdout=asyncio.subprocess.PIPE)
