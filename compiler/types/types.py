from dataclasses import dataclass


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
class Trigger:
    component: str
    method: str


@dataclass
class Argument:
    component: str
    method: str


@dataclass
class Action:
    component: str
    method: str
    args: dict[str, Argument | str]


@dataclass
class Event:
    trigger: Trigger
    do: list[Action]


@dataclass
class State:
    name: str
    events: list[Event]
    bounds: Bounds


@dataclass
class InitialState:
    target: str
    position: Point


@dataclass
class Variable:
    component: str
    method: str


@dataclass
class Condition:
    type: str
    value: Variable | list["Condition"]


@dataclass
class Transition:
    color: str
    condition: Condition | None
    source: str
    target: str
    position: Point
    trigger: Trigger
    do: list[Action]


@dataclass
class Component:
    type: str
    parameters: dict[str, str]


@dataclass
class CompilerSettings:
    filename: str | None
    compiler: str | None
    flags: list[str] | None


@dataclass
class IdeFormat:
    states: dict[str, State]
    initialState: InitialState
    transitions: list[Transition]
    components: dict[str, Component]
    compilerSettings: CompilerSettings | None
    platform: str
    parameters: dict[str, str]
