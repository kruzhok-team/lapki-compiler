"""Config file with information about paths, hosts and ports."""
import os.path
import os
import inspect
from typing import TypeVar

from compiler.types.config_types import ArgumentParser, Config

# НАЧАЛО ПОЛЬЗОВАТЕЛЬСКИХ НАСТРОЕК
_SERVER_PORT = 8081
_SERVER_HOST = 'localhost'
_MODULE_PATH = os.path.dirname(os.path.abspath(inspect.stack()[0][1]))

_ACCESS_TOKENS_FILE = os.path.join(_MODULE_PATH, 'ACCESS_TOKENS.txt')
_BASE_DIRECTORY = _MODULE_PATH + '/'  # "server/"
_BUILD_DIRECTORY = '/tmp/lapki-compiler/'
_LIBRARY_PATH = os.path.join(_MODULE_PATH, 'library/')
_PLATFORM_DIRECTORY = os.path.join(_MODULE_PATH, 'platforms/')
_LOG_PATH = 'logs.log'  # Замените на нужную папку
_MAX_MSG_SIZE = 1024 * 50  # Максимальный размер сообщения от клиента.
# КОНЕЦ ПОЛЬЗОВАТЕЛЬСКИХ НАСТРОЕК
T = TypeVar('T', str, int)


def _update_config() -> Config:
    return Config(library_path=_LIBRARY_PATH,
                  server_host=_SERVER_HOST,
                  platform_directory=_PLATFORM_DIRECTORY,
                  server_port=_SERVER_PORT,
                  max_msg_size=_MAX_MSG_SIZE,
                  log_path=_LOG_PATH,
                  access_token_path=_ACCESS_TOKENS_FILE,
                  build_directory=_BUILD_DIRECTORY,
                  module_directory=_MODULE_PATH,
                  base_path=_BASE_DIRECTORY)


_config = _update_config()


def _choice(flag_arg: str | None,
            env_arg_name: str,
            default_value: T) -> T:
    if flag_arg is not None:
        return default_value.__class__(flag_arg)

    env_arg = os.environ.get(env_arg_name, None)

    if env_arg is not None:
        return default_value.__class__(env_arg)

    return default_value


def get_config() -> Config:
    """Get current compiler configuration."""
    return _config


def configure(parser: ArgumentParser):
    """
    Actualize compiler configuration.

    Priority:
    1) Flags
    2) Environment
    3) config.py
    """
    global _SERVER_PORT
    global _SERVER_HOST
    global _ACCESS_TOKENS_FILE
    global _LIBRARY_PATH
    global _PLATFORM_DIRECTORY
    global _LOG_PATH
    global _MAX_MSG_SIZE
    global _config
    args = parser.parse_args()
    _SERVER_PORT = _choice(
        args.server_port, 'LAPKI_COMPILER_SERVER_PORT', _SERVER_PORT)
    _SERVER_HOST = _choice(
        args.server_host, 'LAPKI_COMPILER_SERVER_HOST', _SERVER_HOST)
    _ACCESS_TOKENS_FILE = _choice(
        args.access_token_path, 'LAPKI_COMPILER_ACCESS_TOKENS_FILE',
        _ACCESS_TOKENS_FILE)
    _LIBRARY_PATH = _choice(
        args.library_path, 'LAPKI_COMPILER_LIBRARY_PATH', _LIBRARY_PATH)
    _PLATFORM_DIRECTORY = _choice(
        args.platform_directory, 'LAPKI_COMPILER_PLATFORM_DIRECTORY',
        _PLATFORM_DIRECTORY)
    _LOG_PATH = _choice(
        args.log_path, 'LAPKI_COMPILER_LOG_PATH', _LOG_PATH)
    _MAX_MSG_SIZE = _choice(
        args.max_msg_size, 'LAPKI_COMPILER_MAX_MSG_SIZE', _MAX_MSG_SIZE)
    _config = _update_config()
