"""Module contains types, that representing unprocessed CyberiadaML scheme."""

from typing import List, Optional

from pydantic.dataclasses import dataclass
from pydantic import Field, ConfigDict


@dataclass
class CGMLRectNode:
    """The type represents <rect> node."""

    x: float = Field(alias='@x')
    y: float = Field(alias='@y')
    width: float = Field(alias='@width')
    height: float = Field(alias='@height')


@dataclass
class CGMLPointNode:
    """The type represents <point> node."""

    x: float = Field(alias='@x')
    y: float = Field(alias='@y')


@dataclass(config=ConfigDict(extra='forbid'))
class CGMLDataNode:
    """The type represents <data> node."""

    key: str = Field(alias='@key')
    content: Optional[str] = Field(default=None, alias='#text')
    rect: Optional[CGMLRectNode] = None
    point: Optional[CGMLPointNode | List[CGMLPointNode]] = None


@dataclass(config=ConfigDict(extra='allow'))
class CGMLKeyNode:
    """The type represents <key> node."""

    id: str = Field(alias='@id')
    for_: str = Field(alias='@for')
    attr_name: Optional[str] = Field(default=None, alias='@attr.name')
    attr_type: Optional[str] = Field(default=None, alias='@attr.type')


@dataclass(config=ConfigDict(extra='forbid'))
class CGMLEdge:
    """The type represents <edge> node."""

    id: str = Field(alias='@id')
    source: str = Field(alias='@source')
    target: str = Field(alias='@target')
    data: Optional[List[CGMLDataNode] | CGMLDataNode] = None


@dataclass(config=ConfigDict(extra='forbid'))
class CGMLGraph:
    """The type represents <graph> node."""

    edgedefault: Optional[str] = Field(alias='@edgedefault', default=None)
    id: Optional[str] = Field(alias='@id', default=None)
    node: Optional[List['CGMLNode'] | 'CGMLNode'] = None
    edge: Optional[List[CGMLEdge] | CGMLEdge] = None
    data: Optional[List[CGMLDataNode] | CGMLDataNode] = None


@dataclass(config=ConfigDict(extra='forbid'))
class CGMLNode:
    """The type represents <node> node."""

    id: str = Field(alias='@id')
    graph: Optional[CGMLGraph | List[CGMLGraph]] = None
    data: List[CGMLDataNode] | CGMLDataNode | None = None


@dataclass(config=ConfigDict(extra='forbid'))
class CGMLGraphml:
    """The type represents <graphml> node."""

    data: CGMLDataNode | List[CGMLDataNode]
    xmlns: str = Field(alias='@xmlns')
    key: Optional[List[CGMLKeyNode] | CGMLKeyNode] = None
    graph: Optional[List[CGMLGraph] | CGMLGraph] = None


@dataclass(config=ConfigDict(extra='forbid'))
class CGML:
    """Root type of CyberiadaML scheme."""

    graphml: CGMLGraphml
