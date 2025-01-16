from typing import Set

import asyncio

async def posix_copy(path_to_libs: Set[str], target_directory: str, cwd: str) -> None:
    await asyncio.create_subprocess_exec('cp',
                                        *path_to_libs,
                                        target_directory,
                                        cwd=cwd
    )
