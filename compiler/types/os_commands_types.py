from typing import Callable, Set, Awaitable, Optional


class OSCommands:
    """Operating system-dependent commands."""

    def __init__(self,
                 copy: Optional[Callable[[Set[str], str, str],
                                         Awaitable[None]]] = None,
                 decode: Optional[Callable[[bytes], str]] = None):
        self._copy = copy
        self._decode = decode

    async def copy(self, path_to_libs: Set[str], target_directory: str, cwd: str) -> None:
        """Copy files from path_to_libs to target directory."""
        if self._copy is None:
            raise Exception('Using OS copy command before initialization!')
        return await self._copy(path_to_libs, target_directory, cwd)

    def decode(self, data: bytes) -> str:
        """Decode byte-data as string."""
        if self._decode is None:
            raise Exception('Using OS copy command before initialization!')
        return self._decode(data)
