"""Legacy module that implements parsing Lapki IDE's internal JSON scheme."""
from typing import Dict, Iterable, List, Set

from compiler.types.ide_types import Bounds
from compiler.types.inner_types import File
from compiler.types.inner_types import (
    DefaultActions,
    EventName,
    EventSignal,
    Events
)
from compiler.fullgraphmlparser.stateclasses import (
    ParserTrigger,
    StateMachine,
    ParserState,
    ParserNote,
    Labels,
    create_note
)
from compiler.types.ide_types import (
    IdeStateMachine,
    State,
    Event,
    Argument,
    Component,
    Trigger,
    Action,
    IncludeStr,
    Transition,
    Condition
)


class ParserException(Exception):
    """Error during parsing CJsonParsinge."""

    ...


class CJsonParser:
    """Class for parsing Lapki IDE's internal JSON scheme."""

    delimeter = {
        'Bearloga': '',
        'arduino-cli': ';',
        'gcc': ';',
        'g++': ';'
    }

    operatorAlias = {
        'notEquals': '!=',
        'equals': '==',
        'greater': '>',
        'less': '<',
        'greaterOrEqual': '>=',
        'lessOrEqual': '<=',
        'or': '||',
        'and': '&&'
    }

    def _initComponent(self, component_name: str, component: Component) -> str:
        """Функция, которая в зависимости от типа компонента\
            возвращает код его инициализации в h-файле."""
        match component.type:
            case 'QHsmSerial':
                return ''
            case _:
                args = ', '.join(
                    map
                    (
                        str,
                        list(component.parameters.values())
                    )
                )
                return (f'\n{component.type} {component_name}'
                        f'= {component.type}'
                        f'({args});')

    def _specificCheckComponentSignal(
            self,
            component_type: str,
            component_name: str,
            trigger: EventSignal,
            signal: str) -> str:
        """
        Функция для специфичных проверок сигналов.

        Так, например, для проверки состояния\
            кнопки необходимо предварительно вызвать функцию scan

        Returns:
            str: специфичная для данного компонента проверка сигнала
        """
        match component_type:
            case 'Timer':
                return f'\n\t{component_name}.timeout();'
            case _:
                return '\n\t\t'.join(
                    [
                        f'\n\t\n\tif({trigger.guard})',
                        '{', f'SIMPLE_DISPATCH(the_sketch, {signal});'
                    ]
                ) + '\n\t}'

    def getLibraries(self, components: List[Component]) -> Set[str]:
        """Получить используемые типы компонентов."""
        libraries: Set[str] = set()
        for component in components:
            if component.type not in libraries:
                libraries.add(f'{component.type}')

        return libraries

    def _setupVariables(
            self,
            component_name: str,
            component: Component) -> str | None:
        """Действия в функции setup."""
        match component.type:
            case 'QHsmSerial':
                args = ', '.join(map(str, list(component.parameters.values())))
                return f'{component_name}::init({args});'
            case 'DigitalOut':
                return f'{component_name}.init();'
            case _:
                return None

    def _actionInMain(
            self,
            component_name: str,
            component: Component) -> str | None:
        """Действия, которые должны происходить каждый тик."""
        match component.type:
            case 'AnalogIn':
                return f'\n\t{component_name}.read();'
            case 'Button':
                return f'\n\t{component_name}.scan();'
            case 'QHsmSerial':
                return '\n\tQHsmSerial::read();'
            case _:
                return None

    def _createNotes(
            self,
            components: Dict[str, Component],
            triggers: Dict[EventName, EventSignal],
            compiler: str) -> List[ParserNote]:
        """Сгенерировать код для вставки."""
        includes: List[IncludeStr] = []
        variables: List[str] = []  # Создание компонентов в h-файле
        components_types: Dict[str, str] = {}  # Название компонента -> его тип
        types: List[str] = []  # Типы, h-файлы которых уже есть в includes
        # Действия, которые будут добавлены в функцию setup
        setup_variables: List[str] = []
        check_signals: List[str] = []  # Действия для добавления в main функцию

        for component_name in components:
            component: Component = components[component_name]
            components_types[component_name] = component.type
            if component.type not in types:
                includes.append(f'\n#include "{component.type}.h"')
                types.append(component.type)

            main_action: str | None = self._actionInMain(
                component_name, component)
            if main_action is not None:
                check_signals.append(main_action)
            setup_variable: str | None = self._setupVariables(
                component_name, component)
            if setup_variable is not None:
                setup_variables.append(setup_variable)
            variables.append(self._initComponent(component_name, component))
        notes: List[ParserNote] = []
        class_filename = 'Sketch'

        for eventName in list(triggers.keys()):
            component_name = triggers[eventName].component_name
            component_type = components_types[component_name]
            check = self._specificCheckComponentSignal(
                component_type,
                component_name,
                triggers[eventName],
                eventName
            )
            check_signals.append(check)

        match compiler:
            case 'g++' | 'gcc':
                setup_function = '\n\t'.join(['\nvoid setup(){',
                                              '\n}'])

                loop_function = ''.join(['\nvoid loop(){', *check_signals,
                                        '\n}'])

                main_function = '\n\t'.join(['\nint main(){',
                                            f'{class_filename}_ctor();',
                                             'QEvt event;',
                                             'QMsm_init(the_sketch, &event);',
                                             'setup();',
                                             'while(true){',
                                             '\tloop();',
                                             '}']) + '\n}'
                notes.extend(
                    [
                        create_note(Labels.H, ''.join(variables)),
                        create_note(Labels.H_INCLUDE, ''.join(includes)),
                        create_note(Labels.CPP, '\n\n'.join(
                            [setup_function, loop_function, main_function]
                        )
                        ),
                    ]
                )

            case 'arduino-cli':
                setup_function = '\n\t'.join(['\nvoid setup(){',
                                              *setup_variables,
                                              f'{class_filename}_ctor();',
                                              'QEvt event;',
                                              'QMsm_init(the_sketch, &event);',
                                              '\n}'])
                loop_function = ''.join(
                    ['\nvoid loop(){', *check_signals, '\n}'])

                notes.extend(
                    [
                        create_note(Labels.H, ''.join(variables)),
                        create_note(Labels.H_INCLUDE, ''.join(includes)),
                        create_note(Labels.CPP, '\n\n'.join(
                            [setup_function, loop_function]
                        )
                        ),
                    ]
                )
            case _:
                ...
        return notes

    def _getCondition(self, condition: Condition, compiler: str) -> str:
        """Рекурсивная функция для генерации условий события."""
        if condition.type in list(CJsonParser.operatorAlias.keys()):
            values: List[str] = []
            if isinstance(condition.value, list):
                for value in condition.value:
                    values.append(self._getCondition(value, compiler=compiler))
            return f' { CJsonParser.operatorAlias[condition.type] } '.join(
                map(str, values))
        elif condition.type == 'value':
            return str(condition.value)
        elif (condition.type == 'component' and
              not isinstance(condition.value, Iterable)):
            component = condition.value.component + '.'
            method = condition.value.method

            # В Берлоге в условиях используются
            # только числа и поля класса!
            args = ''
            arr_args: List[str] = []

            if condition.value.args is not None:
                arr_args = list(condition.value.args.values())

                if len(arr_args) > 0:
                    args = '(' + ','.join(map(str, arr_args)) + ')'
                elif compiler != 'BearlogaDefend':
                    args = '()'

            return ''.join([component, method, args])
        return 'true'

    def _getActions(self, actions: List[Action], compiler: str) -> str:
        """Генерация кода, который будет\
            выполняться при наступлении события."""
        result: List[str] = []
        for action in actions:
            component = action.component
            if component == 'QHsmSerial':
                method = '::' + action.method
            else:
                method = '.' + action.method
            arr_args: List[str] = []
            if action.args is not None:
                for act in list(action.args.values()):
                    if isinstance(act, str):
                        arr_args.append(act)
                    else:
                        arr_args.append(f'{act.component}.{act.method}')
            args = '(' + ','.join(map(str, arr_args)) + ')' + \
                CJsonParser.delimeter[compiler]
            result.append(''.join([component, method, args]))

        return '\n'.join(result)

    def _getTransitions(
            self,
            transitions: List[Transition],
            compiler: str
    ) -> tuple[list[ParserTrigger], dict[EventName, EventSignal]]:
        triggers: List[ParserTrigger] = []
        player_signals: Dict[EventName, EventSignal] = {}
        for i, transition in enumerate(transitions):
            if transition.trigger.component != 'User':
                if transition.trigger.component == 'QHsmSerial':
                    guard = ''.join([transition.trigger.component, '::',
                                     transition.trigger.method, '('])
                else:
                    guard = ''.join([transition.trigger.component, '.',
                                     transition.trigger.method, '('])
                arr_args: List[str] = []
                if transition.trigger.args is not None:
                    arr_args = list(transition.trigger.args.values())
                    guard += ','.join(arr_args)
                guard += ')'

                eventname = ''.join(
                    [
                        transition.trigger.component,
                        '_',
                        transition.trigger.method
                    ]
                ) + '_'.join(arr_args)
                player_signals[eventname] = EventSignal(
                    guard=guard,
                    component_name=transition.trigger.component
                )
                if transition.condition is not None:
                    root: Condition = transition.condition
                    condition = self._getCondition(root, compiler)
                else:
                    condition = 'true'
                if transition.do is not None:
                    action = self._getActions(transition.do, compiler)
                else:
                    action = ''

                triggers.append(ParserTrigger(name=eventname,
                                              source=transition.source,
                                              target=transition.target,
                                              id=str(i),
                                              type='external',
                                              guard=condition,
                                              action=action))
        return (triggers, player_signals)

    def _getEvents(
            self,
            events: List[Event],
            compiler: str,
            state_id: str) -> Events:
        """Получение событий состояния."""
        result: Dict[EventName, ParserTrigger] = {}
        id = 0
        event_signals: Dict[EventName, EventSignal] = {}
        system_signals: Dict[DefaultActions, str] = {
            'onEnter': '',
            'onExit': '',
        }
        for event in events:
            trigger: Trigger = event.trigger
            component = trigger.component
            method = trigger.method
            actions = ''
            for i in range(len(event.do)):
                if event.do[i].component != 'QHsmSerial':
                    actions += event.do[i].component + \
                        '.' + event.do[i].method + '('
                else:
                    actions += event.do[i].component + \
                        '::' + event.do[i].method + '('
                args: Dict[str, Argument | str] | None = event.do[i].args
                if args is not None:
                    arr_action: List[str] = []
                    for arg in list(args.values()):
                        if type(arg) is str:
                            if (event.do[i].component == 'User' and
                                    event.do[i].method == 'emit'):
                                arr_action.append(f'User_{arg}_SIG')
                            else:
                                arr_action.append(arg)
                        elif type(arg) is Argument:
                            if arg.component == 'QHsmSerial':
                                arr_action.append(
                                    f'{arg.component}::{arg.method}')
                            else:
                                arr_action.append(
                                    f'{arg.component}.{arg.method}')
                    actions += ','.join(map(str, arr_action))
                actions += ')' + CJsonParser.delimeter[compiler] + '\n'
            if (component == 'System' and
                    (method == 'onEnter' or method == 'onExit')):
                system_signals[method] = actions
            else:
                eventname = component + '_' + method
                if component == 'QHsmSerial':
                    guard = ''.join([component, '::', method, '()'])
                else:
                    guard = ''.join([component, '.', method, '()'])
                event_signals[eventname] = EventSignal(
                    guard=guard, component_name=trigger.component)
                id += 1
                result[eventname] = ParserTrigger(
                    name=eventname,
                    type='internal',
                    source=state_id,
                    target='',
                    action=actions,
                    id=str(id),
                )
        return Events(events=result,
                      signals=event_signals,
                      system_events=system_signals)

    def _addParentsAndChilds(
            self,
            ide_states: Dict[str, State],
            parser_states: Dict[str, ParserState],
            global_state: ParserState) -> Dict[str, ParserState]:
        """Добавить родителей состояниям."""
        new_states: Dict[str, ParserState] = parser_states.copy()
        for statename in ide_states:
            state: State = ide_states[statename]
            if state.parent is not None:
                new_states[statename].parent = new_states[state.parent]
                new_states[state.parent].childs.append(new_states[statename])
            else:
                new_states[statename].parent = global_state
                global_state.childs.append(new_states[statename])

        return new_states

    def _addTransitionsToStates(
            self,
            transitions: List[ParserTrigger],
            states: Dict[str, ParserState]) -> Dict[str, ParserState]:
        """Привязать переходы к состояниям."""
        new_states: dict[str, ParserState] = states.copy()
        for transition in transitions:
            new_states[transition.source].trigs.append(
                transition)
        return new_states

    def _addSignals(self,
                    components: Dict[str, Component],
                    player_signals: Dict[str, EventSignal]) -> list[str]:
        """
        Добавить специфичные обязательные сигналы.

        Эта функция нужна, чтобы добавить обязательные\
            для работы компонента сигналы, пусть даже они\
                и не используются пользователем напрямую.
        """
        types: set[str] = set()
        signals: List[str] = []
        for component_name in components:
            component: Component = components[component_name]
            match component.type:
                case 'Timer':
                    if (component.type not in types and
                        f'{component_name}_timeout'
                            not in list(player_signals.keys())):
                        signals.append(f'{component_name}_timeout')
                case _:
                    ...
        return signals

    def parseStateMachine(self, data: IdeStateMachine) -> StateMachine:
        """
        Цель данной функции - перевод машины состояний из формата Lapki IDE в\
            формат, принимаемый библиотекой fullgraphmlparser.

        Используется как для Arduino, так и для Берлоги

        Args:
            data (IdeStateMachine): начальные данные
        """
        # Создаем главное, корневое состояние,
        # которое является parent всем состояниям
        global_state = ParserState(name='global', type='group',
                                   actions='', trigs=[],
                                   entry='', exit='',
                                   id='global', new_id=['global'],
                                   parent=None, childs=[],
                                   bounds=Bounds(
                                       x=0,
                                       y=0,
                                       height=0,
                                       width=0
                                   )
                                   )
        states: Dict[str, State] = data.states
        proccesed_states: Dict[str, ParserState] = {}
        event_signals: Dict[EventName, EventSignal] = {}
        compiler: str = 'Bearloga'
        if data.compilerSettings is not None:
            compiler = data.compilerSettings.compiler
        # Инициализация и первичная обработка состояний,
        # не включающая указание родителя.
        for state_id in states:
            state: State = data.states[state_id]
            state_events: Events = self._getEvents(
                state.events, compiler, state_id)
            event_signals = state_events.signals | event_signals

            on_enter = state_events.system_events['onEnter']
            on_exit = state_events.system_events['onExit']
            triggers: Dict[EventName,
                           ParserTrigger] = state_events.events
            proccesed_states[state_id] = ParserState(
                bounds=state.bounds,
                name=state.name,
                type='internal',
                actions='',
                trigs=list(
                    triggers.values()),
                entry=on_enter,
                exit=on_exit,
                id=state_id,
                new_id=[state_id],
                parent=None,
                childs=[])
        transitions, player_signals = self._getTransitions(
            data.transitions, compiler)
        player_signals: Dict[EventName,
                             EventSignal] = player_signals | event_signals
        if compiler in ['arduino-cli', 'g++', 'gcc']:
            notes: List[ParserNote] = self._createNotes(
                data.components, player_signals, compiler)
        else:
            notes = []
        startNode: str = proccesed_states[data.initialState.target].id
        proccesed_states = self._addTransitionsToStates(
            transitions, proccesed_states)
        proccesed_states = self._addParentsAndChilds(
            states, proccesed_states, global_state)
        return StateMachine(
            name='sketch',
            states=[global_state, *list(proccesed_states.values())],
            notes=notes,
            start_action='',
            signals=set([*player_signals.keys(),
                         *self._addSignals(data.components, player_signals)]),
            start_node=startNode,
            main_file_extension='ino',
        )

    def getFiles(self, json_data):  # type: ignore
        """Получение списка SourceFile из Json-объекта."""
        files = []

        for data in json_data:
            files.append(File(
                filename=data['filename'],
                extension=data['extension'],
                fileContent=data['fileContent']))

        return files
