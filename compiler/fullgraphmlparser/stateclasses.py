from typing import List, Optional, Set, Protocol, runtime_checkable
from enum import Enum

from pydantic import Field, BaseModel, ConfigDict
from pydantic.dataclasses import dataclass


def create_note(label: 'Labels', content: str) -> 'ParserNote':
    """
    Создать ParserNote на основе метки вставки, и кода для вставки.

    Между label и контентом добавляется \\n, так как по этому символу\
        сплитится строка в функции write_to_file.
    """
    return ParserNote(
        umlNote=_ParserNoteNodeLabel(
            nodeLabel=_ParserNoteNodeContent(
                text=f'{label.value}:\n{content}')
        )
    )


class Labels(Enum):
    """В fullgraphmlparser для определения, \
        куда вставлять код используют метки."""

    H_INCLUDE = 'Code for h-file'
    H = 'Declare variable in h-file'
    CPP = 'Code for cpp-file'
    CTOR = 'Constructor code'
    SETUP = 'Setup function in cpp-file'
    USER_VAR_H = 'User variables for h-file'
    USER_VAR_C = 'User variables for c-file'
    USER_FUNC_H = 'User methods for h-file'
    USER_FUNC_C = 'User methods for c-file'


@runtime_checkable
class GeometryBounds(Protocol):
    x: float
    y: float
    height: Optional[float]
    width: Optional[float]


class _ParserNoteNodeContent(BaseModel):
    text: str = Field(serialization_alias='#text')


class _ParserNoteNodeLabel(BaseModel):
    nodeLabel: _ParserNoteNodeContent = Field(
        serialization_alias='y:NodeLabel')


class ParserNote(BaseModel):
    """Class for code inserting."""

    umlNote: _ParserNoteNodeLabel = Field(serialization_alias='y:UMLNoteNode')


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
            check_function: function, that check this signal
    """

    name: str
    source: str
    target: str
    action: str
    id: str
    type: str = ''
    guard: str = 'true'
    check_function: str | None = None


@dataclass(config=ConfigDict(arbitrary_types_allowed=True))
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
