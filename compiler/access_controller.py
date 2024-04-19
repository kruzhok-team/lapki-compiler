"""Module implements access controlling."""
import uuid
from typing import Optional, Set

from compiler.config import ACCESS_TOKENS_FILE
from aiofile import async_open


class AccessControllerException(Exception):
    """Error with access."""

    ...


class AccessController:
    """
    Class-singletone, that controls access to platforms.

    Right now every token is admin token with access to all platforms.
    """

    _instance: Optional['AccessController'] = None
    _initialized: bool = False

    def __init__(self) -> None:
        if not self._initialized:
            self.__tokens: Set[str] = set()
            self._initialized = True

    def __new__(cls, *args, **kwargs) -> 'AccessController':
        """
        Class-singletone, that controls access to platforms.

        Return the only one instance.
        """
        if cls._instance is None:
            cls._instance = super().__new__(cls, *args, **kwargs)
        return cls._instance

    async def init_access_tokens(self):
        """Read tokens from ACCESS_TOKENS_FILE."""
        async with async_open(ACCESS_TOKENS_FILE, 'r') as f:
            data: str = await f.read()
            self.__tokens.update(data.split('\n'))

    def create_token(self) -> str:
        """Create new access token."""
        token = uuid.uuid4().hex
        self.__tokens.add(token)
        return token

    def check_access_token(self, token: str) -> bool:
        """Check, that token exist."""
        return token in self.__tokens
