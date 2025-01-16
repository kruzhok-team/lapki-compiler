import os
from compiler.types.os_commands_types import OSCommands
from compiler.os_commands.posix import posix_copy
from compiler.os_commands.nt import nt_copy

os_commands: OSCommands = OSCommands()

def init_os_commands() -> None:
    global os_commands
    match os.name:
        case 'nt':
            os_commands.copy = nt_copy
        case 'posix':
            os_commands.copy = posix_copy
        case _:
            raise Exception(f'Unsupported OS {os.name}!')
