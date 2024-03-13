from typing import Literal, TypeAlias, Optional, List, Dict

from pydantic import BaseModel, model_validator, field_validator
from pydantic.dataclasses import dataclass

Platform: TypeAlias = Literal['BearlogaDefend', 'ArduinoUno']
SupportedCompilers: TypeAlias = Literal['gcc', 'g++', 'arduino-cli']
IncludeStr: TypeAlias = str  # include "blabla.h"


class IDESchemaValidationError(Exception):
    ...


@dataclass
class Point:
    x: float
    y: float


@dataclass
class Bounds:
    x: float
    y: float
    height: float
    width: float


@dataclass
class Argument:
    component: str
    method: str


@dataclass
class Trigger:
    component: str
    method: str
    args: Optional[Dict[str, str]] = None


@dataclass
class Action:
    component: str
    method: str
    args: Optional[Dict[str, Argument | str]] = None


@dataclass
class Event:
    trigger: Trigger
    do: List[Action]


@dataclass
class State:
    name: str
    events: List[Event]
    bounds: Bounds
    parent: Optional[str] = None


@dataclass
class InitialState:
    target: str
    position: Point


@dataclass
class Variable:
    component: str
    method: str
    args: Optional[Dict[str, str]] = None


@dataclass
class Condition:
    type: str
    value: Variable | List['Condition']


@dataclass
class Transition:
    color: str
    source: str
    target: str
    position: Point
    trigger: Trigger
    do: Optional[List[Action]] = None
    condition: Optional[Condition] = None

    @field_validator('color')
    @classmethod
    def _isColor(cls, v: str) -> str:
        if not v.startswith('#') or len(v) != 6:
            raise IDESchemaValidationError(f'{v} - не является цветом!')

        return v


@dataclass
class Component:
    type: str
    parameters: Dict[str, str]


@dataclass
class CompilerSettings:
    filename: str
    compiler: SupportedCompilers
    flags: List[str]


class IdeStateMachine(BaseModel):
    states: Dict[str, State]
    initialState: InitialState
    transitions: List[Transition]
    components: Dict[str, Component]
    compilerSettings: Optional[CompilerSettings] = None
    platform: Platform
    parameters: Dict[str, str]

    @model_validator(mode='after')
    def is_compiler_settings_required(self):
        if self.platform == 'ArduinoUno' and self.compilerSettings is None:
            raise IDESchemaValidationError(
                'Отсутствуют настройки компилятора!')
        if self.platform == 'BearlogaDefend' and self.compilerSettings is not None:
            raise IDESchemaValidationError(
                'Для данной платформы не требуются настройки компилятора!')
        return self
