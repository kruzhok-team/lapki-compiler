import sys
from aiologger.handlers.files import AsyncFileHandler
from aiologger.loggers.json import JsonLogger
from aiologger.levels import LogLevel
from aiologger.formatters.base import Formatter

try:
    from .config import LOG_PATH
except ImportError:
    from compiler.config import LOG_PATH


class Logger:
    logger: JsonLogger

    @staticmethod
    def init_logger():
        Logger.logger = JsonLogger(name="logger", level=LogLevel.DEBUG,
                                   serializer_kwargs={"ensure_ascii": False})

        formatter = Formatter(
            '[%(asctime)s] %(filename)s -> %(funcName)s line:%(lineno)d [%(levelname)s] %(message)s')
        handler = AsyncFileHandler(filename=LOG_PATH)
        handler.formatter = formatter
        Logger.logger.add_handler(handler)

    @staticmethod
    async def logException():
        exception_type, exception_object, exception_traceback = sys.exc_info()
        file = exception_traceback.tb_frame.f_code.co_filename
        line_number = exception_traceback.tb_lineno
        await Logger.logger.exception(f"Exception type: {exception_type}, Exception args: {exception_object.args} File name: {file} , Line number: {line_number}", exc_info=False)
