from typing import Dict, TypeAlias, Literal, List, Optional, Set

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
    checkMethod: str = ''
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
    buildFiles: List[str] = Field(default_factory=list)
    importFiles: List[str] = Field(default_factory=list)
    singletone: bool = False
    constructorParameters: Dict[str, ClassParameter] = Field(
        default_factory=dict)
    initializationParameters: Dict[str, ClassParameter] | None = None
    initializationFunction: str | None = None
    loopActions: List[str] = Field(default_factory=list)


@dataclass
class CompilingSettings:
    command: SupportedCompilers
    flags: List[str]


class Platform(BaseModel):
    id: str = ''
    name: str
    description: str = ''
    compile: bool
    author: str = ''
    icon: str = ''
    format_version: str = Field(alias='formatVersion')
    standard_version: str = Field(alias='standardVersion')
    version: str
    staticComponents: bool
    language: str = ''
    delimeter: str
    visual: bool
    defaultIncludeFiles: List[str] = Field(default_factory=list)
    defaultBuildFiles: Set[str] = Field(default_factory=set)
    compilingSettings: List[CompilingSettings] | None = (
        None
    )
    components: Dict[str, Component]
    mainFunction: bool = False
    mainFileExtension: str = ''


@dataclass
class PlatformMeta:
    """Class contains available versions\
        and tokens to access update platform."""

    versions: Set[str] = Field(default_factory=set)
    name: str = ''
    author: str = ''
