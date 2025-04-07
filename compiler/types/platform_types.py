from typing import Dict, TypeAlias, Literal, List, Optional, Set

from pydantic import BaseModel, Field
from pydantic.dataclasses import dataclass
from compiler.types.ide_types import SupportedCompilers

SupportLanguages: TypeAlias = Literal['C++', 'C']
ParameterType: TypeAlias = str | List[str] | List[int]


@dataclass
class MethodParameter:
    name: str
    description: Optional[str] = None
    type: Optional[ParameterType] = None
    valueAlias: Optional[List[str]] = None


@dataclass
class Signal:
    img: str
    description: str
    checkMethod: str = ''
    parameters: Optional[List[MethodParameter]] = None
    alias: Optional[str] = None


@dataclass
class Variable:
    description: str
    img: str = ''
    type: Optional[str] = None
    alias: Optional[str] = None
    valueAlias: Optional[List[str]] = None


@dataclass
class ClassParameter:
    type: ParameterType
    description: str
    optional: bool = False
    valueAlias: Optional[List[str]] = None


@dataclass
class Method:
    img: str
    description: str
    parameters: List[MethodParameter] = Field(default_factory=list)
    static: bool = False
    alias: Optional[str] = None


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
    name: Optional[str] = None


@dataclass
class CompilingSettings:
    command: SupportedCompilers
    flags: List[str]


@dataclass
class SetupFunction:
    functionName: str
    args: List[str]


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
    static_components: bool = Field(alias='staticComponents')
    component_declaration: bool = Field(
        alias='componentDeclaration', default=False)
    language: str = ''
    delimeter: str
    visual: bool
    default_include_files: List[str] = Field(
        default_factory=list, alias='defaultIncludeFiles')
    default_build_files: Set[str] = Field(
        default_factory=set, alias='defaultBuildFiles')
    compiling_settings: List[CompilingSettings] | None = Field(
        default=None,
        alias='compilingSettings'
    )
    default_setup_functions: List[SetupFunction] = Field(
        default_factory=list, alias='defaultSetupFunctions')
    components: Dict[str, Component]
    main_function: bool = Field(
        default=False, alias='mainFunction')
    main_file_extension: str = Field(
        default='', alias='mainFileExtension')
    header_file_extension: str = Field(
        default='', alias='headerFileExtension')


@dataclass
class PlatformMeta:
    """Class contains available versions\
        and tokens to access update platform."""

    versions: Set[str] = Field(default_factory=set)
    name: str = ''
    author: str = ''
