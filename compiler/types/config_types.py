"""Module contains Config class."""
from dataclasses import dataclass

from tap import Tap
import argcomplete


@dataclass
class Config:
    """Compiler configuration."""

    library_path: str
    server_host: str
    platform_directory: str
    server_port: int
    max_msg_size: int
    log_path: str
    access_token_path: str
    build_directory: str
    module_directory: str
    base_path: str
    KILLABLE: bool = False


class ArgumentParser(Tap):
    """Class describes compiler by flags and env."""

    library_path: str | None = None
    server_host: str | None = None
    platform_directory: str | None = None
    server_port: str | None = None
    max_msg_size: str | None = None
    log_path: str | None = None
    access_token_path: str | None = None
    build_path: str | None = None

    def configure(self):
        """Add CLI args to parser."""
        self.add_argument('--server-port', help='Server port.', required=False)
        self.add_argument('--server-host', help='Server host.', required=False)
        self.add_argument(
            '--library-path', help='Path to directory, '
            'that contain platform sources.', required=False)
        self.add_argument('--platform-directory', help='Path to directory, '
                          'that contain platform json schemes.',
                          required=False
                          )
        self.add_argument('--build-directory', help='The path to the directory'
                          ' where the build'
                          ' will take place',
                          required=False
                          )
        self.add_argument(
            '--log-path', help='Path to log file.', required=False
        )
        self.add_argument('--access-token-path',
                          help='Path to file with access tokens.',
                          required=False
                          )
        self.add_argument('--max-msg-size',
                          help='Max websocket message size.', required=False)
        argcomplete.autocomplete(self)
