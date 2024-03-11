from dataclasses import dataclass
from typing import Literal, Dict, TypeAlias

from ..fullgraphmlparser.stateclasses import ParserTrigger

DefaultComponents = Literal['System']
DefaultActions = Literal['onEnter', 'onExit']

EventName: TypeAlias = str


@dataclass
class EventSignal:
    guard: str
    component_name: str


@dataclass
class Events:
    """EventName - строка вида User_component_method, или component_method."""

    events: Dict[EventName, ParserTrigger]
    user_events: Dict[EventName, ParserTrigger]
    system_events: Dict[DefaultActions, str]
    signals: Dict[EventName, EventSignal]
