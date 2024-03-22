"""Module implements timer."""

import asyncio
from types import FunctionType
from typing import Any


class Timer:
    """
    Реализация таймера.

    Принимает на вход время таймаута, асинхронную функцию и ее аргументы.
    Заготовлена для использования в PlatformManager.
    """

    def __init__(self, timeout: int | float,
                 callback: FunctionType,
                 args: tuple[Any]) -> None:
        self._timeout: int | float = timeout
        self._callback: FunctionType = callback
        self._args: tuple[Any] = args

    async def _work(self) -> None:
        await asyncio.sleep(self._timeout)
        await self._callback(*self._args)

    def start(self) -> None:
        """Start task."""
        self._task = asyncio.create_task(self._work())

    def cancel(self) -> None:
        """Cancel task."""
        self._task.cancel()

    def restart(self) -> None:
        """Restart task."""
        self.cancel()
        self.start()
