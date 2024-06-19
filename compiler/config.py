"""Config file with information about paths, hosts and ports."""
import os.path
import os
import inspect
from typing import TypeVar

from compiler.types.config_types import ArgumentParser

SERVER_PORT = 8081
SERVER_HOST = 'localhost'
MODULE_PATH = os.path.dirname(os.path.abspath(inspect.stack()[0][1]))

ACCESS_TOKENS_FILE = os.path.join(MODULE_PATH, 'ACCESS_TOKENS.txt')
BASE_DIRECTORY = MODULE_PATH + '/'  # "server/"
BUILD_DIRECTORY = '/tmp/lapki-compiler/'
LIBRARY_PATH = os.path.join(MODULE_PATH, 'library/')
PLATFORM_DIRECTORY = os.path.join(MODULE_PATH, 'platforms/')
LOG_PATH = 'logs.log'  # Замените на нужную папку
MAX_MSG_SIZE = 1024 * 50  # Максимальный размер сообщения от клиента.

T = TypeVar('T', str, int)


def _choice(flag_arg: str | None,
            env_arg_name: str,
            default_value: T) -> T:
    if flag_arg is not None:
        return default_value.__class__(flag_arg)

    env_arg = os.environ.get(env_arg_name, None)

    if env_arg is not None:
        return default_value.__class__(env_arg)

    return default_value


def configure(parser: ArgumentParser):
    """
    Actualize compiler configuration.

    Priority:
    1) Flags
    2) Environment
    3) config.py
    """
    global SERVER_PORT
    global SERVER_HOST
    global ACCESS_TOKENS_FILE
    global LIBRARY_PATH
    global PLATFORM_DIRECTORY
    global LOG_PATH
    global MAX_MSG_SIZE
    args = parser.parse_args()
    SERVER_PORT = _choice(args.server_port, 'SERVER_PORT', SERVER_PORT)
    SERVER_HOST = _choice(args.server_host, 'SERVER_HOST', SERVER_HOST)
    ACCESS_TOKENS_FILE = _choice(
        args.access_token_path, 'ACCESS_TOKENS_FILE', ACCESS_TOKENS_FILE)
    LIBRARY_PATH = _choice(
        args.library_path, 'LIBRARY_PATH', LIBRARY_PATH)
    PLATFORM_DIRECTORY = _choice(
        args.platform_directory, 'PLATFORM_DIRECTORY', PLATFORM_DIRECTORY)
    LOG_PATH = _choice(
        args.log_path, 'LOG_PATH', LOG_PATH)
    MAX_MSG_SIZE = _choice(
        args.max_msg_size, 'MAX_MSG_SIZE', MAX_MSG_SIZE)
