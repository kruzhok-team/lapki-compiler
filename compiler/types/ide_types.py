from dataclasses import dataclass
from typing import Literal, TypeAlias, Optional
from collections.abc import Mapping, Sequence

from pydantic import BaseModel, model_validator


Platform: TypeAlias = Literal['BearlogaDefend', 'ArduinoUno']


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
    args: Mapping[str, Argument | str]


@dataclass
class Event(BaseModel):
    trigger: Trigger
    do: Sequence[Action]


@dataclass
class State(BaseModel):
    name: str
    events: Sequence[Event]
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
    value: Variable | Sequence["Condition"]


@dataclass
class Transition(BaseModel):
    color: str
    condition: Optional[Condition]
    source: str
    target: str
    position: Point
    trigger: Trigger
    do: Sequence[Action]


@dataclass
class Component(BaseModel):
    type: str
    parameters: Mapping[str, str]


@dataclass
class CompilerSettings(BaseModel):
    filename: str
    compiler: str
    flags: Sequence[str]


class IdeFormat(BaseModel):
    states: Mapping[str, State]
    initialState: InitialState
    transitions: Sequence[Transition]
    components: Mapping[str, Component]
    compilerSettings: Optional[CompilerSettings] = None
    platform: Platform
    parameters: Mapping[str, str]

    class Config:
        require_by_default = False

    @model_validator(mode='after')
    def is_compiler_settings_required(self):
        if self.platform == 'ArduinoUno' and self.compilerSettings == None:
            raise IDESchemaValidationError('Отсутствуют настройки компилятора!')
        if self.platform == 'BearlogaDefend' and self.compilerSettings != None:
            raise IDESchemaValidationError(
                'Для данной платформы не требуются настройки компилятора!')
        return self
