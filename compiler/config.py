"""Config file with information about paths, hosts and ports."""
import os.path
import inspect
import argparse
import argcomplete

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


def setup_args(parser: argparse.ArgumentParser):
    """Add CLI args to parser."""
    parser.add_argument('--server-port', help='Server port.')
    parser.add_argument('--server-host', help='Server host.')
    parser.add_argument(
        '--library-path', help='Path to directory, '
        'that contain platform sources.')
    parser.add_argument('--platform-direcory', help='Path to directory, '
                        'that contain platform json schemes.')
    parser.add_argument('--log-path', help='Path to log file.')
    parser.add_argument('--max-msg-size', help='Max websocket message size.')
    argcomplete.autocomplete(parser)


def configure(parser: argparse.ArgumentParser):
    """
    Actualize compiler configuration.

    Priority:
    1) Flags
    2) Environment
    3) config.py
    """
    ...
