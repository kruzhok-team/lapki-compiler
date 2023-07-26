import os.path
import inspect

SERVER_PORT = 8081

MODULE_PATH = os.path.dirname(os.path.abspath(inspect.stack()[0][1]))

BASE_DIRECTORY = MODULE_PATH + "/"  # "server/"
BUILD_DIRECTORY = "/tmp/lapki-compiler/"
LIBRARY_SOURCE_PATH = os.path.join(MODULE_PATH, "library/source/")
LIBRARY_BINARY_PATH = os.path.join(MODULE_PATH, "library/build/")
SCHEMA_DIRECTORY = BASE_DIRECTORY + "schemas/"
