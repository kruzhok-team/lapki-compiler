from typing import Dict, TypeAlias, Literal, List, Optional

from pydantic import BaseModel


ParameterType: TypeAlias = Literal['int', 'str', 'uint8_t', 'byte', 'unsigned int',
                                   'unsigned long', 'char[]', 'int | char[]']


class Signal(BaseModel):
    img: str
    description: str


class Variable(BaseModel):
    img: str
    description: str
    type: Optional[ParameterType] = None


class ClassParameter(BaseModel):
    type: ParameterType
    description: str


class MethodParameter(BaseModel):
    description: Optional[str] = None
    name: str
    type: Optional[ParameterType | List[str]] = None


class Method(BaseModel):
    img: str
    parameters: List[MethodParameter] = []
    description: str


class Component(BaseModel):
    description: str
    img: str
    signals: Dict[str, Signal]
    variables: Dict[str, Variable]
    methods: Dict[str, Method]
    parameters: Dict[str, ClassParameter]


class Platform(BaseModel):
    name: str
    components: Dict[str, Component]
    variables: Dict[str, Variable] = {}
    singletone: bool = False


class UnprocessedPlatform(BaseModel):
    platform: Dict[str, Platform]
