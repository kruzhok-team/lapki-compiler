from typing import TypeAlias, Literal

from pydantic.dataclasses import dataclass


Message: TypeAlias = Literal['close',
                             'berlogaImport', 'arduino', 'berlogaExport']


@dataclass
class IMG:
    filepath: str
    content: bytes
