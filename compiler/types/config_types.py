"""Module contains Config class."""
from tap import Tap
import argcomplete


class ArgumentParser(Tap):
    """Class describes compiler by flags and env."""

    library_path: str | None = None
    server_host: str | None = None
    platform_directory: str | None = None
    server_port: str | None = None
    max_msg_size: str | None = None
    log_path: str | None = None
    access_token_path: str | None = None

    def configure(self):
        """Add CLI args to parser."""
        self.add_argument('--server-port', help='Server port.', required=False)
        self.add_argument('--server-host', help='Server host.', required=False)
        self.add_argument(
            '--library-path', help='Path to directory, '
            'that contain platform sources.', required=False)
        self.add_argument('--platform-direcory', help='Path to directory, '
                          'that contain platform json schemes.', required=False)
        self.add_argument(
            '--log-path', help='Path to log file.', required=False)
        self.add_argument('--access-token-path',
                          help='Path to file with access tokens.', required=False)
        self.add_argument('--max-msg-size',
                          help='Max websocket message size.', required=False)
        argcomplete.autocomplete(self)
