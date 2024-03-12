"""Legacy module that implements parsing Lapki IDE's internal JSON scheme."""

from enum import Enum
from typing import Dict, Iterable, List, Set


try:
    from .SourceFile import SourceFile
    from .fullgraphmlparser.stateclasses import (
        ParserTrigger,
        StateMachine,
        ParserState,
        ParserNote,
        ParserNoteNodeLabel,
        ParserNoteNodeContent
    )
    from .types.ide_types import (
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
    from .types.inner_types import (
        DefaultActions,
        EventName,
        EventSignal,
        Events
    )
except ImportError:
    from compiler.types.inner_types import (
        DefaultActions,
        EventName,
        EventSignal,
        Events
    )
    from compiler.SourceFile import SourceFile
    from compiler.fullgraphmlparser.stateclasses import (
        ParserTrigger,
        StateMachine,
        ParserState,
        ParserNote,
        ParserNoteNodeLabel,
        ParserNoteNodeContent
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


class Labels(Enum):
    """В fullgraphmlparser для определения, \
        куда вставлять код используют метки."""

    H_INCLUDE = 'Code for h-file'
    H = 'Declare variable in h-file'
    CPP = 'Code for cpp-file'
    CTOR = 'Constructor code'
    USER_VAR_H = 'User variables for h-file'
    USER_VAR_C = 'User variables for c-file'
    USER_FUNC_H = 'User methods for h-file'
    USER_FUNC_C = 'User methods for c-file'


class CJsonParser:
    """Class for parsing Lapki IDE's internal JSON scheme."""

    delimeter = {
        'Berloga': '',
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

    def initComponent(self, component_name: str, component: Component) -> str:
        """Функция, которая в зависимости от типа компонента\
            возвращает код его инициализации в h-файле."""
        match component.type:
            case 'Timer':
                return (f'\n{type} {component_name} = {type}'
                        f'(the_sketch,{component_name}_timeout_SIG);')
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
                return (f'\n{type} {component_name} = {type}'
                        f'({args});')

    def specificCheckComponentSignal(
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

    def getNote(self, label: Labels, content: str) -> ParserNote:
        """Создать ParserNote на основе метки вставки, и кода для вставки."""
        return ParserNote(
            umlNote=ParserNoteNodeLabel(
                nodeLabel=ParserNoteNodeContent(
                    text=f'{label.value}: {content}')
            )
        )

    def getLibraries(self, components: List[Component]) -> Set[str]:
        """Получить используемые типы компонентов."""
        libraries: Set[str] = set()
        for component in components:
            if component.type not in libraries:
                libraries.add(f'{component.type}')

        return libraries

    def setupVariables(
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

    def actionInMain(
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

    def createNotes(
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

            main_action: str | None = self.actionInMain(
                component_name, component)
            if main_action is not None:
                check_signals.append(main_action)
            setup_variable: str | None = self.setupVariables(
                component_name, component)
            if setup_variable is not None:
                setup_variables.append(setup_variable)
            variables.append(self.initComponent(component_name, component))
        notes: List[ParserNote] = []
        class_filename = 'Sketch'

        for eventName in list(triggers.keys()):
            component_name = triggers[eventName].component_name
            component_type = components_types[component_name]
            check = self.specificCheckComponentSignal(component_name,
                                                      component_type,
                                                      triggers[eventName],
                                                      eventName)
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
                        self.getNote(Labels.H, ''.join(variables)),
                        self.getNote(Labels.H_INCLUDE, ''.join(includes)),
                        self.getNote(Labels.CPP, '\n\n'.join(
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
                        self.getNote(Labels.H, ''.join(variables)),
                        self.getNote(Labels.H_INCLUDE, ''.join(includes)),
                        self.getNote(Labels.CPP, '\n\n'.join(
                            [setup_function, loop_function]
                        )
                        ),
                    ]
                )
            case _:
                ...
        return notes

    def getCondition(self, condition: Condition, compiler: str) -> str:
        """Рекурсивная функция для генерации условий события."""
        if condition.type in list(CJsonParser.operatorAlias.keys()):
            values: List[str] = []
            if isinstance(condition.value, Iterable):
                for value in condition.value:
                    values.append(self.getCondition(value, compiler=compiler))
            return f' { CJsonParser.operatorAlias[condition.type] } '.join(
                map(str, values))
        elif condition.type == 'value':
            return str(condition.value)
        elif condition.type == 'component' and not isinstance(condition.value, Iterable):
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

    def getActions(self, actions: List[Action], compiler: str) -> str:
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

    def getTransitions(self, transitions: List[Transition], compiler: str) -> tuple[list[ParserTrigger], dict[EventName, EventSignal]]:
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

                eventname = ''.join([transition.trigger.component, '_',
                                     transition.trigger.method]) + '_'.join(arr_args)
                player_signals[eventname] = EventSignal(
                    guard=guard,
                    component_name=transition.trigger.component
                )
                if transition.condition is not None:
                    root: Condition = transition.condition
                    condition = self.getCondition(root, compiler)
                else:
                    condition = 'true'
                if transition.do is not None:
                    action = self.getActions(transition.do, compiler)
                else:
                    action = ''

                triggers.append(ParserTrigger(name=eventname, source=transition.source,
                                              target=transition.target, id=i,
                                              type='external', guard=condition,
                                              action=action))
        return (triggers, player_signals)

    def getEvents(self, events: List[Event], compiler: str, state_id: str) -> Events:
        result: Dict[EventName, ParserTrigger] = {}
        id = 0
        event_signals: Dict[EventName, EventSignal] = {}
        system_signals: Dict[DefaultActions, str] = {
            'onEnter': '',
            'onExit': '',
        }
        user_events: Dict[EventName, ParserTrigger] = {}
        for event in events:
            trigger: Trigger = event.trigger
            component = trigger.component
            method = trigger.method
            actions = ''
            for i in range(len(event.do)):
                if component != 'User':
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
                            if event.do[i].component == 'User' and event.do[i].method == 'emit':
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
            if component == 'System' and (method == 'onEnter' or method == 'onExit'):
                system_signals[method] = actions
            elif component == 'User':
                eventname = 'User_' + method
                trig = ParserTrigger(name=eventname,
                                     type='internal',
                                     source=state_id,
                                     target='',
                                     action=actions,
                                     id=id,
                                     )
                user_events[eventname] = trig
                id += 1
            else:
                eventname = component + '_' + method
                if component == 'QHsmSerial':
                    guard = ''.join([component, '::', method, '()'])
                else:
                    guard = ''.join([component, '.', method, '()'])
                event_signals[eventname] = EventSignal(
                    guard=guard, component_name=trigger.component)
                id += 1
                result[eventname] = ParserTrigger(name=eventname, type='internal', source=state_id,
                                                  target='', action=actions, id=id,
                                                  )
        return Events(events=result,
                      signals=event_signals,
                      system_events=system_signals,
                      user_events=user_events)

    def addParentsAndChilds(self, ide_states: Dict[str, State], parser_states: Dict[str, ParserState], global_state: ParserState) -> Dict[str, ParserState]:
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

    def addTransitionsToStates(self, transitions: List[ParserTrigger], states: Dict[str, ParserState]):
        new_states: dict[str, ParserState] = states.copy()
        for transition in transitions:
            new_states[transition.source].trigs.append(
                transition)
        return new_states

    def addSignals(self, components: Dict[str, Component], player_signals: Dict[str, EventSignal]) -> list[str]:
        types: set[str] = set()
        signals: List[str] = []
        for component_name in components:
            component: Component = components[component_name]
            match component.type:
                case 'Timer':
                    if component.type not in types and f'{component_name}_timeout' not in list(player_signals.keys()):
                        signals.append(f'{component_name}_timeout')
                case _:
                    ...
        return signals

    def parseStateMachine(self, data: IdeStateMachine) -> StateMachine:
        """ Цель данной функции - перевод машины состояний из формата Lapki IDE в формат, 
        принимаемый библиотекой fullgraphmlparser.

        Используется как для Arduino, так и для Берлоги

        Args:
            json_data (IdeStateMachine): начальные данные
            ws (WebSocketResponse): вебсокет для вызова ошибок
            class_name (Optional[str], optional): название класса
        """
        # Создаем главное, корневое состояние, которое является parent всем состояниям
        global_state = ParserState(name='global', type='group',
                                   actions='', trigs=[],
                                   entry='', exit='',
                                   id='global', new_id=['global'],
                                   parent=None, childs=[])
        states: Dict[str, State] = data.states
        proccesed_states: Dict[str, ParserState] = {}
        event_signals: Dict[EventName, EventSignal] = {}
        # Инициализация и первичная обработка состояний, не включающая указание родителя.
        compiler: str = 'Bearloga'
        if data.compilerSettings is not None:
            compiler = data.compilerSettings.compiler
        for state_id in states:
            state: State = data.states[state_id]
            state_events: Events = self.getEvents(
                state.events, compiler, state_id)
            event_signals = state_events.signals | event_signals

            on_enter = state_events.system_events['onEnter']
            on_exit = state_events.system_events['onExit']
            triggers: Dict[EventName,
                           ParserTrigger] = state_events.events | state_events.user_events
            proccesed_states[state_id] = ParserState(name=state.name, type='state',
                                                     actions='',
                                                     trigs=list(
                                                         triggers.values()),
                                                     entry=on_enter,
                                                     exit=on_exit,
                                                     id=state_id,
                                                     new_id=[state_id],
                                                     parent=None,
                                                     childs=[])
        transitions, player_signals = self.getTransitions(
            data.transitions, compiler)
        player_signals: Dict[EventName,
                             EventSignal] = player_signals | event_signals
        if compiler in ['arduino-cli', 'g++', 'gcc']:
            notes: List[ParserNote] = self.createNotes(
                data.components, player_signals, compiler)
        else:
            notes = []
        startNode: str = proccesed_states[data.initialState.target].id
        proccesed_states = self.addTransitionsToStates(
            transitions, proccesed_states)
        proccesed_states = self.addParentsAndChilds(
            states, proccesed_states, global_state)
        return StateMachine(
            name='sketch',
            states=[global_state, *list(proccesed_states.values())],
            notes=notes,
            start_action='',
            signals=[*player_signals.keys(),
                     *self.addSignals(data.components, player_signals)],
            start_node=startNode
        )

    def getFiles(self, json_data):  # type: ignore
        files = []

        for data in json_data:
            files.append(SourceFile(
                data['filename'], data['extension'], data['fileContent']))

        return files
