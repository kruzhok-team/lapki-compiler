from typing import List, Optional, Set, Protocol

from pydantic import Field, BaseModel
from pydantic.dataclasses import dataclass

from compiler.types.ide_types import Bounds, IdeStateMachine


class GeometryBounds(Protocol):
    x: float
    y: float
    height: Optional[float]
    width: Optional[float]


class ParserNoteNodeContent(BaseModel):
    text: str = Field(serialization_alias='#text')


class ParserNoteNodeLabel(BaseModel):
    nodeLabel: ParserNoteNodeContent = Field(serialization_alias='y:NodeLabel')


class ParserNote(BaseModel):
    umlNote: ParserNoteNodeLabel = Field(serialization_alias='y:UMLNoteNode')


@dataclass
class ParserTrigger:
    """
    Class Trigger describes Triggers of uml-diagrams
            name: name of trigger
            type: internal or external
            guard: text of trigger guard if any
            source: source state of trigger (actual for external triggers)
            target: target state of trigger (actual for external triggers)
            action: action for this trigger if any
            id: order number of internal trigger for better coordinates
            x, y: start of trigger visual path
            dx, dy: first relative movement of trigger visual path
            points: other relative movements of trigger visual path
            action_x, action_y, action_width: coordinates of trigger label
    """

    name: str
    source: str
    target: str
    action: str
    id: int
    type: str = ''
    guard: str = 'true'


@dataclass
class ParserState:
    """
    class State describes state of uml-diagram and trigslates to qm format.
    Fields:
            name: name of state
            type: state or choice
            trigs: list of trigsitions from this state both external and internal
            entry: action on entry event
            exit: action on exit event
            id: number of state
            actions: raw_data for external actions
            old_id: id of state in graphml
            x, y: graphical coordinates
            height, width: height and with of node
    """
    name: str
    type: str
    actions: str
    trigs: List[ParserTrigger]
    entry: str
    exit: str
    id: str
    new_id: List[str]
    parent: Optional['ParserState']
    childs: List['ParserState']
    bounds: GeometryBounds

    def __str__(self) -> str:
        if self.parent is not None:
            return f"{self.name, self.parent.name, ', '.join([child.name for child in self.childs]) }"
        else:
            return f"{self.name}, parent: None, {', '.join([child.name for child in self.childs])}"


@dataclass
class StateMachine:
    name: str
    start_node: str
    start_action: str
    notes: List[ParserNote]
    states: List[ParserState]
    signals: Set[str]
