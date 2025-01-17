from typing import Callable, Set, Awaitable, Optional

class OSCommands:
    """
    Operating system-dependent commands.
    """
    def __init__(self, copy: Optional[Callable[[Set[str], str, str], Awaitable[None]]] = None):
        self._copy = copy
        
    async def copy(self, path_to_libs: Set[str], target_directory: str, cwd: str) -> None:
        if self._copy is None:
            raise Exception('Using OS copy command before initialization!')
        return await self._copy(path_to_libs, target_directory, cwd)
    
