from pydantic import BaseModel
from typing import Dict, TypeAlias, Literal, List, Optional

ParameterType: TypeAlias = Literal['int', 'str', 'uint8_t', 'byte', 'unsigned int',
                                   'unsigned long', 'char[]', 'int | char[]']


class Signal(BaseModel):
    img: str
    description: str


class Variable(BaseModel):
    img: str
    description: str
    type: Optional[ParameterType]


class Parameter(BaseModel):
    description: str
    type: ParameterType | List[str]


class Method(BaseModel):
    img: str
    parameters: List[Parameter]


class Component(BaseModel):
    description: str
    img: str
    signals: Dict[str, Signal]
    variables: Dict[str, Variable]
    methods: Dict[str, Method]
    parameters: Dict[str, Parameter]


class Platform(BaseModel):
    name: str
    components: Dict[str, Component]
    parameters: Dict[str, Parameter]
    variables: Dict[str, Variable]
    singletone: bool


class UnprocessedPlatform(BaseModel):
    platform: Dict[str, Platform]
