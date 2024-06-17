"""Module implements inner compiler's types."""

from typing import List, Literal, Dict, Optional, TypeAlias, Any

from pydantic import BaseModel, field_validator
from pydantic.dataclasses import dataclass
from compiler.fullgraphmlparser.stateclasses import ParserTrigger

DefaultComponents = Literal['System']
DefaultActions = Literal['onEnter', 'onExit']
StateMachineId = str
EventName: TypeAlias = str


@dataclass
class InnerComponent:
    type: str
    parameters: Dict[str, Any]


@dataclass
class InnerTrigger:
    """Dataclass represents trigger[condition] postfix/ string."""

    trigger: Optional[str]
    condition: Optional[str]
    postfix: Optional[str]


@dataclass
class InnerEvent:
    """
    Dataclass represents parsed event string.

    check - function, that check signal
    event/ actions
    """

    event: InnerTrigger
    actions: str
    check: str | None = None


class File(BaseModel):
    filename: str
    extension: str
    fileContent: str | bytes

    @field_validator('filename', mode='after')
    @classmethod
    def check_file_path(cls, v: str) -> str:
        """Check, that filepath doesn't is not relative."""
        if '..' in v:
            raise ValueError('Path is not correct, remove all .. from path.')
        return v


class StateMachineResult(BaseModel):
    """State machine compiling result."""

    result: str
    return_code: int
    stdout: str
    stderr: str
    binary: List[File]
    source: List[File]


class CompilerResponse(BaseModel):
    """
    Data, that compiler send.

    result: compilation status, if at least one state \
        machine has not been compiled, then the value will be NOTOK.
    """

    result: str
    state_machines: Dict[str, StateMachineResult]


@dataclass
class EventSignal:
    """
    Служебный класс.

    guard - условие события.
    component_name - компонент, вызывающий событие.
    """

    guard: str
    component_name: str


@dataclass
class Events:
    """
    Класс для хранения событий, полученных из _getEvents.

    EventName - строка вида User_component_method, или component_method.
    system_events - стандартные события, такие как onEnter и onExit,
    signals: сформированные сигналы
        ключ - в fullgraphmlparser требуется список всех событий.
        значение - информация, которая требуется для самого CJsonParser.
    """

    events: Dict[EventName, ParserTrigger]
    system_events: Dict[DefaultActions, str]
    signals: Dict[EventName, EventSignal]
