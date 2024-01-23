from dataclasses import dataclass
from typing import Literal, TypeAlias, Optional, List, Dict

from pydantic import BaseModel, model_validator, field_validator


Platform: TypeAlias = Literal['BearlogaDefend', 'ArduinoUno']
Compiler: TypeAlias = Literal['gcc', 'g++', 'arduino-cli']


class IDESchemaValidationError(Exception):
    ...


@dataclass
class Point(BaseModel):
    x: float
    y: float


@dataclass
class Bounds(BaseModel):
    x: float
    y: float
    height: float
    width: float


@dataclass
class Trigger(BaseModel):
    component: str
    method: str


@dataclass
class Argument(BaseModel):
    component: str
    method: str


@dataclass
class Action(BaseModel):
    component: str
    method: str
    args: Dict[str, Argument | str]


@dataclass
class Event(BaseModel):
    trigger: Trigger
    do: List[Action]


@dataclass
class State(BaseModel):
    name: str
    events: List[Event]
    bounds: Bounds


@dataclass
class InitialState(BaseModel):
    target: str
    position: Point


@dataclass
class Variable(BaseModel):
    component: str
    method: str


@dataclass
class Condition(BaseModel):
    type: str
    value: Variable | List["Condition"]


@dataclass
class Transition(BaseModel):
    color: str
    source: str
    target: str
    position: Point
    trigger: Trigger
    do: List[Action]
    condition: Optional[Condition] = None

    @field_validator('color')
    @classmethod
    def isColor(cls, v: str) -> str:
        if not v.startswith('#') or len(v) != 6:
            raise IDESchemaValidationError(f'{v} - не является цветом!')

        return v


@dataclass
class Component(BaseModel):
    type: str
    parameters: Dict[str, str]


@dataclass
class CompilerSettings(BaseModel):
    filename: str
    compiler: Compiler
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
            raise IDESchemaValidationError('Отсутствуют настройки компилятора!')
        if self.platform == 'BearlogaDefend' and self.compilerSettings is not None:
            raise IDESchemaValidationError(
                'Для данной платформы не требуются настройки компилятора!')
        return self
