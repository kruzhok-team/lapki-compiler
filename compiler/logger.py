"""Module implements logging."""
import sys

from aiopath import AsyncPath
from aiologger.handlers.files import AsyncFileHandler
from aiologger.loggers.json import JsonLogger
from aiologger.levels import LogLevel
from aiologger.formatters.base import Formatter
from compiler.config import get_config


class Logger:
    """Class for logging."""

    logger: JsonLogger

    @staticmethod
    async def init_logger() -> None:
        """Initializize and configure logger."""
        config = get_config()
        await AsyncPath(config.log_path[:config.log_path.rfind('/')]).mkdir(
            parents=True,
            exist_ok=True)
        await AsyncPath(config.log_path).touch(exist_ok=True)
        Logger.logger = JsonLogger(name='logger', level=LogLevel.DEBUG,
                                   serializer_kwargs={'ensure_ascii': False})

        formatter = Formatter(
            '[%(asctime)s] %(filename)s -> %(funcName)s\
                line:%(lineno)d [%(levelname)s] %(message)s')
        handler = AsyncFileHandler(filename=config.log_path)
        handler.formatter = formatter
        Logger.logger.add_handler(handler)

    @staticmethod
    async def logException() -> None:
        """Write exception traceback and exception\
            object information to log file."""
        exception_type, exception_object, exception_traceback = sys.exc_info()
        error = ('Exception occured, but traceback'
                 'and exception object is None!')

        if exception_traceback is not None:
            print(exception_traceback.tb_next)
            file: str = exception_traceback.tb_frame.f_code.co_filename
            line_number: int = exception_traceback.tb_lineno
            error = (f'Exception type: {exception_type},'
                     f'File name: {file} , Line number: {line_number}')
        if exception_object is not None:
            # Logger.logger.exception('asdasdasdasds')
            error += f'Exception args: {exception_object.args}'

        await Logger.logger.exception(error, exc_info=True)
