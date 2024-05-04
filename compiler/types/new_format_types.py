"""Module contains the types of parsed scheme's elements."""
from typing import (
    List,
    Dict,
    DefaultDict,
    Literal,
    Optional,
    TypeAlias
)

from pydantic import Field
from pydantic.dataclasses import dataclass

try:
    from .cgml_scheme import CGMLDataNode, CGMLKeyNode
    from .common import Point, Rectangle
except ImportError:
    from cyberiadaml_py.types.cgml_scheme import CGMLDataNode, CGMLKeyNode
    from cyberiadaml_py.types.common import Point, Rectangle
#  { node: ['dGeometry', ...], edge: ['dData', ...]}
AvailableKeys: TypeAlias = DefaultDict[str, List[CGMLKeyNode]]

CGMLVertexType = Literal['choice', 'initial', 'final', 'terminate']
CGMLNoteType = Literal['formal', 'informal']


@dataclass
class CGMLBaseVertex:
    """
    The type represents pseudo-nodes.

    type: content from nested <data>-node with key 'dVertex'.
    data: content from nested <data>-node with key 'dName'.
    position: content from nested <data>-node with key 'dGeometry'.
    parent: parent node id.
    """

    type: CGMLVertexType
    data: Optional[str] = None
    position: Optional[Point | Rectangle] = None
    parent: Optional[str] = None


@dataclass
class CGMLState:
    """
    Data class with information about state.

    State is <node>, that not connected with meta node,\
        doesn't have data node with key 'dNote'

    Parameters:
    name: content of data node with key 'dName'.
    actions: content of data node with key 'dData'.
    bounds: x, y, width, height properties of data node with key 'dGeometry'.
    parent: parent state id.
    color: content of data node with key 'dColor'.
    unknownDatanodes: all datanodes, whose information\
        is not included in the type.
    """

    name: str
    actions: str
    unknown_datanodes: List[CGMLDataNode]
    parent: Optional[str] = None
    bounds: Optional[Rectangle | Point] = None
    color: Optional[str] = None


@dataclass
class CGMLComponent:
    """
    Data class with information about component.

    Component is formal note, that includes <data>-node with key 'dName'\
        and content 'CGML_COMPONENT'.
    parameters: content of data node with key 'dData'.
    """

    id: str
    type: str
    parameters: Dict[str, str]


@dataclass
class CGMLInitialState(CGMLBaseVertex):
    """
    Data class with information about initial state (pseudo node).

    Intiial state is <node>, that contains data node with key 'dVertex'\
        and content 'initial'.
    """

    ...


@dataclass
class CGMLChoice(CGMLBaseVertex):
    """
    Data class with information about choice node (pseudo node).

    Choice is <node>, that contains data node with key 'dVertex'\
        and content 'choice'.
    """

    ...


@dataclass
class CGMLTransition:
    """
    Data class with information about transition(<edge>).

    Parameters:
    source: <edge> source property's content.
    target: <edge> target property's content.
    actions: content of data node with 'dData' key.
    color: content of data node with 'dColor' key.
    position: x, y properties of data node with 'dGeometry' key.
    unknownDatanodes: all datanodes, whose information\
        is not included in the type.
    """

    id: str
    source: str
    target: str
    actions: str
    unknown_datanodes: List[CGMLDataNode]
    color: Optional[str] = None
    position: List[Point] = Field(default_factory=list)
    label_position: Optional[Point] = None
    pivot: Optional[str] = None


@dataclass
class CGMLNote:
    """
    Dataclass with infromation about note.

    Note is <node> containing data node with key 'dNote'
    type: content of <data key="dNote">
    text: content of <data key="dData">
    name: content of <data key="dName">
    position: properties <data key="dGeometry">'s child\
        <point> or <rect>
    unknownDatanodes: all datanodes, whose information\
        is not included in the type.
    """

    name: str
    position: Point | Rectangle
    text: str
    type: str
    unknown_datanodes: List[CGMLDataNode]
    parent: str | None = None


@dataclass
class CGMLMeta:
    """
    The type represents meta-information from formal\
        note with 'dName' CGML_META.

    id: meta-node id.
    values: information from meta node, exclude required parameters.
    """

    id: str
    values: Dict[str, str]


@dataclass
class CGMLFinal(CGMLBaseVertex):
    """
    The type represents final-states.

    Final state - <node>, that includes dVertex\
        with content 'final'.
    """

    ...


@dataclass
class CGMLTerminate(CGMLBaseVertex):
    """
    The type represents terminate-states.

    Final state - <node>, that includes dVertex\
        with content 'terminate'.
    """

    ...


@dataclass
class CGMLElements:
    """
    Dataclass with elements of parsed scheme.

    Contains dict of CGMLStates, where the key is state's id.
    Also contains trainstions, components, awaialable keys, notes.

    States doesn't contains components nodes and pseudo-nodes.
    Transitions doesn't contains edges from meta-node(<node id=''>)\
        to components nodes.

    Parameters:
    meta: content of data node\
        with key 'dData' inside meta-node.
    format: content of data node with key 'gFormat'.
    platform: content of meta-data
    keys: dict of KeyNodes, where the key is 'for' attribute.\
        Example: { "node": [KeyNode, ...], "edge": [...] }
    """

    states: Dict[str, CGMLState]
    transitions: Dict[str, CGMLTransition]
    components: Dict[str, CGMLComponent]
    standard_version: str
    platform: str
    meta: CGMLMeta
    format: str
    keys: AvailableKeys
    notes: Dict[str, CGMLNote]
    initial_states: Dict[str, CGMLInitialState]
    finals: Dict[str, CGMLFinal]
    choices: Dict[str, CGMLChoice]
    terminates: Dict[str, CGMLTerminate]


Vertex = CGMLFinal | CGMLChoice | CGMLInitialState | CGMLTerminate
