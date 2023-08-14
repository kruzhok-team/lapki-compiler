import asyncio
from types import FunctionType


class Timer:
    """
        Реализация таймера. Принимает на вход время таймаута,
        асинхронную функцию и ее аргументы.
        Заготовлена для использования в PlatformManager.
    """
    def __init__(self, timeout: int | float,
                 callback: FunctionType,
                 args: tuple):
        self._timeout = timeout
        self._callback = callback
        self._args = args

    async def _work(self):
        await asyncio.sleep(self._timeout)
        await self._callback(*self._args)

    def start(self):
        self._task = asyncio.create_task(self._work())

    def cancel(self):
        self._task.cancel()

    def restart(self):
        self.cancel()
        self.start()
