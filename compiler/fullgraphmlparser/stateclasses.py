from typing import (
    List,
    Optional,
    Set,
    Protocol,
    runtime_checkable,
    Literal
)
from enum import Enum

from pydantic import Field, BaseModel, ConfigDict
from pydantic.dataclasses import dataclass
from compiler.types.platform_types import CompilingSettings

TriggerType = Literal['internal', 'external', 'choice_start', 'choice_result']
StateType = Literal['group', 'choice', 'internal']
VertexType = Literal['final', 'initial', 'choice', 'terminate']


def create_note(label: 'Labels', content: str) -> 'ParserNote':
    """
    Создать ParserNote на основе метки вставки, и кода, который нужно вставить.

    Между label и контентом добавляется \\n, так как по этому символу\
        сплитится строка в функции write_to_file.
    """
    return ParserNote(
        umlNote=_ParserNoteNodeLabel(
            nodeLabel=_ParserNoteNodeContent(
                text=f'{label.value}:\n{content}')
        )
    )


class Labels(Enum):
    """В fullgraphmlparser для определения, \
        куда вставлять код используют метки."""

    H_INCLUDE = 'Code for h-file'
    H = 'Declare variable in h-file'
    CPP = 'Code for cpp-file'
    CTOR = 'Constructor code'
    SETUP = 'Setup function in cpp-file'
    LOOP = 'Loop function in cpp-file'
    USER_VAR_H = 'User variables for h-file'
    USER_VAR_C = 'User variables for c-file'
    USER_FUNC_H = 'User methods for h-file'
    USER_FUNC_C = 'User methods for c-file'
    CTOR_FIELDS = 'Constructor fields'
    STATE_FIELDS = 'State fields'
    EVENT_FIELDS = 'Event fields'
    MAIN_FUNCTION = 'Main function'


@runtime_checkable
class Condition(Protocol):
    guard: str
    action: str


@runtime_checkable
class GeometryBounds(Protocol):
    x: float
    y: float
    height: Optional[float]
    width: Optional[float]


class _ParserNoteNodeContent(BaseModel):
    text: str = Field(serialization_alias='#text')


class _ParserNoteNodeLabel(BaseModel):
    nodeLabel: _ParserNoteNodeContent = Field(
        serialization_alias='y:NodeLabel')


class ParserNote(BaseModel):
    """
    Class for code inserting.

    ### Create only by create_note function.
    """

    umlNote: _ParserNoteNodeLabel = Field(serialization_alias='y:UMLNoteNode')


@dataclass
class ParserTrigger:
    """
    Class Trigger describes Triggers of uml-diagrams.

            name: name of trigger
            type: internal or external
            guard: text of trigger guard if any
            source: source state of trigger (actual for external triggers)
            target: target state of trigger (actual for external triggers)
            action: action for this trigger if any
            id: order number of internal trigger for better coordinates
            x, y: start of trigger visual path
            dx, dy: first relative movement of trigger visual path
            points: other relative movements of trigger visual path
            action_x, action_y, action_width: coordinates of trigger label
            check_function: function, that check this signal
            defer: delayed signal emmiting, if true, then the\
                actions will not be recorded
    """

    name: str
    source: str
    target: str
    action: str
    id: str
    type: TriggerType = 'internal'
    guard: str = 'true'
    check_function: str | None = None
    defer: bool = False
    propagate: bool = False


@dataclass
class UnconditionalTransition:
    """
    Безусловные переходы.

    Свойства:
    - Не нужно проверять в loop
    - Нельзя вызвать сигналом
    - Нет ограждающего условия
    - Нет propagate/block
    - Используются в начальных состояниях
    """

    action: str
    target: str


@dataclass
class ChoiceTransition:
    action: str
    target: str
    guard: str = 'else'


@dataclass
class BaseParserVertex:
    """Базовый класс для всех узлов-псевдосостояний."""

    id: str
    parent: str | None


@dataclass
class GeneratorFinalVertex(BaseParserVertex):
    """Класс, обозначающий финальное состояние."""

    ...


@dataclass
class GeneratorInitialVertex(BaseParserVertex):
    """Класс, обозначающий начальное псевдосостояние."""

    transition: UnconditionalTransition


@dataclass
class GeneratorChoiceVertex(BaseParserVertex):
    """Класс, обозначающий псевдосостояние выбора."""

    transitions: List[ChoiceTransition]


@dataclass
class GeneratorShallowHistory(BaseParserVertex):
    """
    Псевдосостояние локальной истории.

    Генерация псевдосостояния истории помимо генерации стандартного вертекса
    состоит из инициализации массива и хранении последних последних посещений.
    ```cpp
    // инициализация
    QStateHandler shallowHistory[3] = {
        Q_STATE_CAST(Sketch_pixtlgycbblxtahjlzhl),
        Q_STATE_CAST(Sketch_pixtlgycbblxtahjlzhl),
        Q_STATE_CAST(QHsm_top) // default value
    };
    ```

    ```cpp
    // Совершили переход и сохранили состояние
    shallowHistory[0] = {  Q_STATE_CAST(Sketch_mlqctlmxugztidzjjhfz) };
    ```
    `index`: индекс в массиве локальных историй.

    `default_value`: идентификатор состояния, в которое ведет переход
    в локальную историю, если на уровне локального
    состояния не было совершено переходов.

    """

    index: int
    default_value: str | None = None


@dataclass(config=ConfigDict(arbitrary_types_allowed=True))
class ParserState:
    """
    class State describes state of uml-diagram and trigslates to qm format.
    Fields:
            name: name of state
            type: state or choice
            trigs: list of trigsitions from this state both external and internal
            entry: action on entry event
            exit: action on exit event
            id: number of state
            actions: raw_data for external actions
            old_id: id of state in graphml
            x, y: graphical coordinates
            height, width: height and with of node
    """

    name: str
    type: StateType
    actions: str
    trigs: List[ParserTrigger]
    entry: str
    exit: str
    id: str
    new_id: List[str]
    parent: Optional['ParserState']
    childs: List['ParserState']
    bounds: Optional[GeometryBounds] = None
    initial_state: Optional[str] = None

    def __str__(self) -> str:
        if self.parent is not None:
            return f"{self.name, self.parent.name, ', '.join([child.name for child in self.childs]) }"
        else:
            return f"{self.name}, parent: None, {', '.join([child.name for child in self.childs])}"


@dataclass
class SMCompilingSettings:
    import_files: List[str]
    build_files: Set[str]
    platform_id: str
    platform_version: str
    platform_compiler_settings: List[CompilingSettings]


@dataclass
class StateMachine:
    """
    Данные машины состояний, на основе которых генерируется кода
    """
    name: str | None
    start_node: str
    start_action: str
    notes: List[ParserNote]
    states: List[ParserState]
    signals: Set[str]
    main_file_extension: str
    header_file_extension: str
    language: str
    # Установлено дефолтное значение, чтобы не трогать легаси.
    id: str = 'sketch'
    initial_states: List[GeneratorInitialVertex] = Field(default_factory=list)
    choices: List[GeneratorChoiceVertex] = Field(default_factory=list)
    final_states: List[GeneratorFinalVertex] = Field(default_factory=list)
    shallow_history: List[GeneratorShallowHistory] = Field(default=list)
    compiling_settings: Optional[SMCompilingSettings] = None


class CodeGenerationException(Exception):
    def __init__(self, error_data: str):
        super().__init__(self)
        self.error_data = error_data

    def __str__(self):
        if self.error_data:
            return str(self.error_data)
        return 'Неизвестная ошибка генерации кода!'

    def __repr__(self):
        return f'CodeGenerationException({repr(self.error_data)})'
