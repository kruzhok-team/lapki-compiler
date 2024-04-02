"""Config file with information about paths, hosts and ports."""
import os.path
import inspect

SERVER_PORT = 8081
SERVER_HOST = 'localhost'
MODULE_PATH = os.path.dirname(os.path.abspath(inspect.stack()[0][1]))

BASE_DIRECTORY = MODULE_PATH + '/'  # "server/"
BUILD_DIRECTORY = '/tmp/lapki-compiler/'
LIBRARY_PATH = os.path.join(MODULE_PATH, 'library/')
PLATFORM_DIRECTORY = os.path.join(MODULE_PATH, 'platforms/')
LOG_PATH = 'logs.log'  # Замените на нужную папку
MAX_MSG_SIZE = 1024 * 50  # Максимальный размер сообщения от клиента.