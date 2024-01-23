from typing import TypeAlias, Literal
from dataclasses import dataclass
from aiohttp.web import WebSocketResponse
Message: TypeAlias = Literal['close', 'berlogaImport', 'arduino', 'berlogaExport']


@dataclass
class File:
    filename: str
    extension: str
    fileContent: str


@dataclass
class WSResponse(WebSocketResponse):
    result: Literal['OK', 'NOTOK']
    return_code: int
    stdout: str
    strerr: str
    binary: File
