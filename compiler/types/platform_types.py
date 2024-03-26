from typing import Dict, TypeAlias, Literal, List, Optional

from pydantic import BaseModel, Field
from pydantic.dataclasses import dataclass
from compiler.types.ide_types import SupportedCompilers

SupportLanguages: TypeAlias = Literal['C++']
ParameterType: TypeAlias = Literal['int', 'str', 'uint8_t', 'byte', 'unsigned int',
                                   'unsigned long', 'char[]', 'int | char[]']


@dataclass
class MethodParameter:
    name: str
    description: Optional[str] = None
    type: Optional[ParameterType | List[str]] = None


@dataclass
class Signal:
    img: str
    description: str
    checkMethod: str
    parameters: Optional[List[MethodParameter]] = None


@dataclass
class Variable:
    description: str
    img: str = ''
    type: Optional[ParameterType] = None


@dataclass
class ClassParameter:
    type: ParameterType
    description: str
    optional: bool = False


@dataclass
class Method:
    img: str
    description: str
    parameters: List[MethodParameter] = Field(default_factory=list)
    static: bool = False


@dataclass
class Component:
    description: str
    img: str
    signals: Dict[str, Signal]
    variables: Dict[str, Variable]
    methods: Dict[str, Method]
    buildFiles: List[str]
    importFiles: List[str]
    singletone: bool = False
    constructorParameters: Dict[str, ClassParameter] = Field(
        default_factory=dict)
    initializationParameters: Dict[str, ClassParameter] | None = None
    initializationFunction: str | None = None
    loopActions: List[str] = Field(default_factory=list)


@dataclass
class CompilingSettings:
    compiler: SupportedCompilers
    flags: List[str]


class Platform(BaseModel):
    name: str
    description: str = ''
    icon: str = ''
    id: str = ''
    staticComponents: bool
    language: str
    author: str = ''
    visual: bool
    compilingSettings: CompilingSettings
    components: Dict[str, Component]
