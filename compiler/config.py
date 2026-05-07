"""Config file with information about paths, hosts and ports."""
import os.path
import os
import inspect
from typing import TypeVar
import sys

from compiler.types.config_types import ArgumentParser, Config

_MODULE_PATH = (
    os.path.dirname(os.path.abspath(inspect.stack()[0][1]))
    if not getattr(sys, 'frozen', False)
    else os.path.join(os.path.dirname(sys.executable))
)
_BASE_DIRECTORY = _MODULE_PATH  # "server/"

# НАЧАЛО ПОЛЬЗОВАТЕЛЬСКИХ НАСТРОЕК
_SERVER_PORT = 8081
_SERVER_HOST = 'localhost'
_ACCESS_TOKENS_FILE = os.path.join(_MODULE_PATH, 'ACCESS_TOKENS.txt')
_BUILD_DIRECTORY = os.path.join(_MODULE_PATH, 'build')
_ARTIFACTS_DIRECTORY = os.path.join(_BUILD_DIRECTORY, 'artifacts')
_LIBRARY_PATH = os.path.join(_MODULE_PATH, 'library')
_PLATFORM_DIRECTORY = os.path.join(_MODULE_PATH, 'platforms')
_LOG_PATH = 'logs.log'  # Замените на нужную папку
_MAX_MSG_SIZE = 1024 * 256  # Максимальный размер сообщения от клиента.
_KILLABLE = False
# КОНЕЦ ПОЛЬЗОВАТЕЛЬСКИХ НАСТРОЕК
T = TypeVar('T', str, int)


def get_default_config() -> Config:
    """Get config with default values."""
    return Config(_LIBRARY_PATH,
                  _SERVER_HOST,
                  _PLATFORM_DIRECTORY,
                  _SERVER_PORT,
                  _MAX_MSG_SIZE,
                  _LOG_PATH,
                  _ACCESS_TOKENS_FILE,
                  _BUILD_DIRECTORY,
                  _MODULE_PATH,
                  _BASE_DIRECTORY,
                  _KILLABLE)


_config = get_default_config()


def _choice(flag_arg: str | bool | None,
            env_arg_name: str,
            default_value: T) -> T:
    if flag_arg is not None:
        if isinstance(default_value, bool):
            return default_value.__class__(str(flag_arg).lower()
                                           in ('1', 'true', 'yes', 'on'))
        return default_value.__class__(flag_arg)

    env_arg = os.environ.get(env_arg_name, None)

    if env_arg is not None:
        if isinstance(default_value, bool):
            return default_value.__class__(str(env_arg).lower()
                                           in ('1', 'true', 'yes', 'on'))
        return default_value.__class__(env_arg)

    return default_value


def set_config(new_config: Config) -> None:
    """Set new configuration."""
    # TODO: Автоматический рестарт с новыми параметрами?
    global _config
    _config = new_config


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
    args = parser.parse_args()
    server_port = _choice(
        args.server_port, 'LAPKI_COMPILER_SERVER_PORT', _SERVER_PORT)
    server_host = _choice(
        args.server_host, 'LAPKI_COMPILER_SERVER_HOST', _SERVER_HOST)
    access_token_file = _choice(
        args.access_token_path, 'LAPKI_COMPILER_ACCESS_TOKENS_FILE',
        _ACCESS_TOKENS_FILE)
    library_path = _choice(
        args.library_path, 'LAPKI_COMPILER_LIBRARY_PATH', _LIBRARY_PATH)
    platform_directory = _choice(
        args.platform_directory, 'LAPKI_COMPILER_PLATFORM_DIRECTORY',
        _PLATFORM_DIRECTORY)
    log_path = _choice(
        args.log_path, 'LAPKI_COMPILER_LOG_PATH', _LOG_PATH)
    max_msg_size = _choice(
        args.max_msg_size, 'LAPKI_COMPILER_MAX_MSG_SIZE', _MAX_MSG_SIZE)
    build_directory = _choice(
        args.build_path, 'LAPKI_COMPILER_BUILD_PATH', _BUILD_DIRECTORY)
    killable = _choice(
        args.killable, 'LAPKI_COMPILER_KILLABLE', _KILLABLE
    )
    set_config(Config(
        library_path,
        server_host,
        platform_directory,
        server_port,
        max_msg_size,
        log_path,
        access_token_file,
        build_directory,
        _MODULE_PATH,
        _BASE_DIRECTORY,
        bool(killable))
    )
