"""The module for initializing OS commands."""
import os

from compiler.types.os_commands_types import OSCommands
from compiler.os_commands.posix import posix_copy, posix_decode
from compiler.os_commands.nt import nt_copy, nt_decode


os_commands: OSCommands = OSCommands()


def init_os_commands() -> None:
    """
    Initialize commands depending on the OS.

    Write commands to os_commands variable.
    """
    global os_commands
    match os.name:
        case 'nt':
            os_commands.copy = nt_copy
            os_commands.decode = nt_decode
        case 'posix':
            os_commands.copy = posix_copy
            os_commands.decode = posix_decode
        case _:
            raise Exception(f'Unsupported OS {os.name}!')
