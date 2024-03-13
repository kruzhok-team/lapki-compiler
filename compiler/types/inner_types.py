"""Module implements inner compiler's types."""

from dataclasses import dataclass
from typing import Literal, Dict, TypeAlias

from ..fullgraphmlparser.stateclasses import ParserTrigger

DefaultComponents = Literal['System']
DefaultActions = Literal['onEnter', 'onExit']

EventName: TypeAlias = str


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
