from enum import Enum
from typing import Dict, Iterable, List, Optional
from aiohttp.web import WebSocketResponse
from compiler.types.ide_types import Action, Transition, Condition
from compiler.types.inner_types import DefaultActions, EventName, EventSignal, Events

from compiler.types.platform_types import Platform

try:
    from .types.ide_types import State, Event, Argument, Component
    from .SourceFile import SourceFile
    from .fullgraphmlparser.stateclasses import ParserTrigger, StateMachine, ParserState
    from .types.ide_types import IdeStateMachine, Trigger
except ImportError:
    from compiler.SourceFile import SourceFile
    from compiler.fullgraphmlparser.stateclasses import ParserTrigger, StateMachine, ParserState
    from compiler.types.ide_types import IdeStateMachine, State, Event, Argument, Component, Trigger

class ParserException(Exception):
    ...

class StateMachineValidatorException(Exception):
    ...

class Labels(Enum):
    H_INCLUDE = 'Code for h-file'
    H = 'Declare variable in h-file'
    CPP = 'Code for cpp-file'
    CTOR = 'Constructor code'
    USER_VAR_H = 'User variables for h-file'
    USER_VAR_C = 'User variables for c-file'
    USER_FUNC_H = 'User methods for h-file'
    USER_FUNC_C = 'User methods for c-file'

class StateMachineValidator:
    
    def __init__(self, data: IdeStateMachine, platform: Platform) -> None:
        self.data = data
        self.platform = platform
    
    def validateComponents(self) -> bool:
        """
        Функция проверяет соответствие компонентов указанной платформе
        """
        component_types = list(self.platform.components.keys())
        for component_id in self.data.components:
            component = self.data.components[component_id]
            if component.type in component_types:
                parameters = list(self.platform.components[component.type].parameters.keys())
                for parameter_id in component.parameters:
                    if parameter_id in parameters:
                        continue
                    raise StateMachineValidatorException(f'Component({component_id}):\
                        unknown parameter {parameter_id} in platform {self.platform.name}')
            raise StateMachineValidatorException(f'Component({component_id}):\
                unknown component {component.type} in platform {self.platform.name}')
        
        return True

    def validateArgs(self, component: Component, method: str, args: Dict[str, Argument | str]) -> bool:
        ...

    def validate(self, event: Event) -> bool:
        ...

class CJsonParser:
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

    
    def initComponent(self, type: str, name: str, parameters: dict, filename: str):
        '''
            Функция, которая в зависимости от компонента
            возвращает код его инициализации в h-файле.
        '''
        match type:
            case 'Timer':
                return f'\n{type} {name} = {type}(the_{filename}, {name}_timeout_SIG);'
            case 'QHsmSerial':
                return ''
            case _:
                return f'\n{type} {name} = {type}({", ".join(map(str, list(parameters.values())))});'

    
    def specificCheckComponentSignal(type: str, name: str, triggers: dict, filename: str, signal: str) -> str:
        '''Функция для специфичных проверок сигналов. Так, например, для
        проверки состояния кнопки необходимо предварительно вызвать функцию scan

        Returns:
            str: специчиная для данного компонента проверка сигнала
        '''
        match type:
            # case 'Button':
            #         return \n\t\n\tif({triggers['guard']})', '{', f'SIMPLE_DISPATCH(the_{filename}, {signal});\n\t']) + '\n\t}'
            case 'Timer':
                return f'\n\t{name}.timeout();'
            # case 'QHsmSerial':
            #  }'   if not checked:
            #         return '\n\t\t'.join([f'\n\t{name}.readByte();\
            #                 \n\t\n\tif({triggers['guard']})', '{', f'SIMPLE_DISPATCH(the_{filename}, {signal});\n\t']) + '\n\t}'
            #     else:
            #         return '\n\t\t'.join([f'\n\t\n\tif({triggers['guard']})', '{', f'SIMPLE_DISPATCH(the_{filename}, {signal});\n\t']) + '\n\t
            case _:
                return '\n\t\t'.join([f'\n\t\n\tif({triggers['guard']})', '{', f'SIMPLE_DISPATCH(the_{filename}, {signal});']) + '\n\t}'

    
    def appendNote(self, label: Labels, content: str, notes: list):
        notes.append({'y:UMLNoteNode':
                      {'y:NodeLabel':
                       {'#text': f'{label.value}: {content}'}}})

    
    def getLibraries(self, components) -> list[str]:
        libraries = []
        for component in components:
            if component.type not in libraries:
                libraries.append(f'{component.type}')

        return libraries

    
    def setupVariables(self, name: str, type: str, parameters: dict) -> str | None:
        match type:
            case 'QHsmSerial':
                return f'{name}::init({', '.join(map(str, list(parameters.values())))});'
            case 'DigitalOut':
                return f'{name}.init();'

        return None

    
    def actionInMain(self, component: Component, signals: list[str]) -> None:
        match component.type:
            case 'AnalogIn':
                signals.append(
                    f'\n\t{component.name}.read();')
            case 'Button':
                signals.append(f'\n\t{component.name}.scan();')
            case 'QHsmSerial':
                signals.append(f'\n\tQHsmSerial::read();')

    
    def createNotes(self, components: list[Component], filename: str, triggers: dict, compiler: str) -> list:
        includes = []
        variables = []
        setup = []
        components_types = {}
        types = []
        setup_variables: list[str] = []
        check_signals = []

        for component in components:
            components_types[component.name] = component.type
            if component.type not in types:
                includes.append(f'\n#include '{component.type}.h'')
                types.append(component.type)

            CJsonParser.actionInMain(component, check_signals)
            setup_variable = CJsonParser.setupVariables(
                component.name, component.type, component.parameters)
            if setup_variable:
                setup_variables.append(setup_variable)
            variables.append(CJsonParser.initComponent(component.type,
                                                       component.name,
                                                       component.parameters,
                                                       filename))
        notes = []
        class_filename = filename[0].upper() + filename[1:]

        for name in triggers.keys():
            component_name = triggers[name]['component_name']
            component_type = components_types[component_name]
            check = CJsonParser.specificCheckComponentSignal(name=component_name,
                                                             type=component_type,
                                                             triggers=triggers[name],
                                                             filename=filename,
                                                             signal=name)
            check_signals.append(check)

        match compiler:
            case 'g++' | 'gcc':
                setup_function = '\n\t'.join(['\nvoid setup(){',
                                              *setup,
                                              '\n}'])

                loop_function = ''.join(['\nvoid loop(){', *check_signals,
                                        '\n}'])

                main_function = '\n\t'.join(['\nint main(){',
                                            f'{class_filename}_ctor();',
                                             'QEvt event;',
                                             f'QMsm_init(the_{filename}, &event);',
                                             'setup();',
                                             'while(true){',
                                             '\tloop();',
                                             '}']) + '\n}'
                CJsonParser.appendNote(Labels.H, ''.join(variables), notes)
                CJsonParser.appendNote(
                    Labels.H_INCLUDE, ''.join(includes), notes)
                CJsonParser.appendNote(Labels.CPP, '\n\n'.join(
                    [setup_function, loop_function, main_function]), notes)

            case 'arduino-cli':
                setup_function = '\n\t'.join(['\nvoid setup(){',
                                              *setup_variables,
                                              f'{class_filename}_ctor();',
                                              'QEvt event;',
                                              f'QMsm_init(the_{filename}, &event);',
                                              '\n}'])
                loop_function = ''.join(
                    ['\nvoid loop(){', *check_signals, '\n}'])
                CJsonParser.appendNote(Labels.H, ''.join(variables), notes)
                CJsonParser.appendNote(
                    Labels.H_INCLUDE, ''.join(includes), notes)
                CJsonParser.appendNote(Labels.CPP, '\n\n'.join(
                    [setup_function, loop_function]), notes)
        return notes

    
    def getComponents(self, components: list) -> list[Component]:
        result = []

        for component_name in components:
            result.append(Component(
                component_name, type=components[component_name]['type'], parameters=components[component_name]['parameters']))

        return result

    
    def getCondition(self, condition: Condition, compiler: str) -> str:
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
        result: List[str] = []
        for action in actions:
            component = action.component
            if component == 'User' or component == 'QHsmSerial':
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

    
    def getTransitions(self, transitions: List[Transition], compiler: str):
        result = []
        user_transitions = []
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
                if transition.do != []:
                    action = self.getActions(transition.do, compiler)
                else:
                    action = ''

                result.append(ParserTrigger(name=eventname, source=transition.source,
                                          target=transition.target, id=i,
                                          type='external', guard=condition,
                                          action=action, points=[]))
            else:
                eventname = f'User_{transition['trigger']['method']}'
                if 'condition' in transition.keys() and transition['condition'] is not None:
                    root = transition['condition']
                    condition = await CJsonParser.getCondition(root, compiler)
                else:
                    condition = 'true'
                if 'do' in transition.keys():
                    action = await CJsonParser.getActions(transition['do'],
                                                          compiler)
                else:
                    action = ''
                trig = Trigger(name=name,
                               source=transition['source'],
                               target=transition['target'], id=i,
                               type='external', guard=condition,
                               action=action, points=[])
                user_transitions.append({
                    'trigger': trig
                })
                i += 1
        return result, player_signals, user_transitions

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
                               points=[])
                user_events[eventname] = trig
                id += 1
            else:
                eventname = component + '_' + method
                if component == 'QHsmSerial':
                    guard = ''.join([component, '::', method, '()'])
                else:
                    guard = ''.join([component, '.', method, '()'])
                event_signals[eventname] = EventSignal(guard=guard, component_name=trigger.component)
                trig = ParserTrigger(name=eventname, type='internal', source=state_id,
                               target='', action=actions, id=id,
                               points=[])
                id += 1
                result[eventname] = trig
        return Events(events=result, 
                      signals=event_signals, 
                      system_events=system_signals, 
                      user_events=user_events)

    
    def addParentsAndChilds(self, states, processed_states, global_state):
        result = processed_states.copy()
        for statename in states:
            state = states[statename]
            try:
                result[statename].parent = result[state['parent']]
                result[state['parent']].childs.append(result[statename])
            except KeyError:
                result[statename].parent = global_state
                global_state.childs.append(result[statename])

        return result

    
    def addTransitionsToStates(self, transitions, states):
        new_states = states.copy()
        for transition in transitions:
            new_states[transition['trigger'].source].trigs.append(
                transition['trigger'])

        return new_states

    
    def addSignals(self, components: list[Component], player_signals: list[str]) -> list[str]:
        types: set[str] = set()
        signals: list[str] = []
        for component in components:
            match component.type:
                case 'Timer':
                    if component.type not in types and f'{component.name}_timeout' not in player_signals:
                        signals.append(f'{component.name}_timeout')
        return signals

    
    def getUserFunctions(self, functions: dict[str, dict]) -> tuple[str, str]:
        h = []
        c = []
        for func_name in list(functions.keys()):
            return_type = functions[func_name]['returnType']

            args = []
            for arg in list(functions[func_name]['args'].keys()):
                name = arg
                arg_type: str = functions[func_name]['args'][arg]['type']
                pos = arg_type.find('[')
                if pos != -1:
                    arg_type = arg_type[:pos]
                    name = name + '[]'
                args.append(f'{arg_type} {name}')
            args = ', '.join(args)
            code = functions[func_name]['code']
            h.append(f'\nstatic {return_type} {func_name}({args});')
            c.append(f'\n{return_type} User::{func_name}({args})' +
                     '{' + f'\n{code}' + '\n}')

        return ('\n'.join(h), '\n'.join(c))

    
    def getUserVariables(self, variables: dict[str, dict[str, str]]) -> tuple[str, str]:
        h = []
        c = []

        for variable_name in list(variables.keys()):
            vtype = variables[variable_name]['type']
            val = variables[variable_name]['value']

            pos = vtype.find('[')
            if pos != -1:
                vtype = vtype[:pos]
                variable_name = variable_name + '[]'

            h.append(f'\nstatic {vtype} {variable_name};')
            c.append(f'\n{vtype} User::{variable_name} = {val};')

        return ('\n'.join(h), '\n'.join(c))

    
    def createUserCode(self, user_data: dict) -> tuple[list[str], list[str]]:
        notes = []
        functions = CJsonParser.getUserFunctions(user_data['functions'])
        if functions != ('', ''):
            CJsonParser.appendNote(Labels.USER_FUNC_H, functions[0], notes)
            CJsonParser.appendNote(Labels.USER_FUNC_C, functions[1], notes)
        variables = CJsonParser.getUserVariables(user_data['variables'])

        if variables != ('', ''):
            CJsonParser.appendNote(Labels.USER_VAR_H, variables[0], notes)
            CJsonParser.appendNote(Labels.USER_VAR_C, variables[1], notes)

        signals = []

        for signal in user_data['signals']:
            signals.append('User_' + signal)

        return notes, signals

    
    def parseStateMachine(self, data: IdeStateMachine, ws: WebSocketResponse, class_name: Optional[str] = None) -> StateMachine:
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
        states = data.states
        proccesed_states: Dict[str, ParserState] = {}
        event_signals: Dict[str, EventSignal] = {}
        # Инициализация и первичная обработка состояний, не включающая указание родителя.
        compiler: str = 'Bearloga'
        if data.compilerSettings is not None:
            compiler = data.compilerSettings.compiler
        for state_id in states:
            state: State = data.states[state_id]
            state_events: Events = self.getEvents(state.events, compiler, state_id)
            event_signals = state_events.signals | event_signals

            on_enter = state_events.system_events['onEnter']
            on_exit = state_events.system_events['onExit']
            triggers: Dict[EventName, ParserTrigger] = state_events.events | state_events.user_events
            proccesed_states[state_id] = ParserState(name=state.name, type='state',
                                                actions='',
                                                trigs=list(triggers.values()),
                                                entry=on_enter, 
                                                exit=on_exit,
                                                id=state_id,
                                                new_id=[state_id],
                                                parent=None,
                                                childs=[])
        transitions, player_signals, user_transitions = self.getTransitions(data.transitions, compiler)
        player_signals = dict(
            list(player_signals.items()) + list(event_signals.items()))
        components = CJsonParser.getComponents(data.components)
        user_signals = []
        if compiler in ['arduino-cli', 'g++', 'gcc']:
            notes = CJsonParser.createNotes(components, filename, triggers=player_signals, compiler=compiler, path=path)
            if 'User' in list(json_data.keys()):
                user_notes, user_signals = CJsonParser.createUserCode(
                    json_data['User'])
                notes = [*notes, *user_notes]
        else:
            notes = []
        startNode = proccesed_states[json_data['initialState']['target']].id
        proccesed_states = await CJsonParser.addTransitionsToStates([*transitions, *user_transitions], proccesed_states)
        proccesed_states = await CJsonParser.addParentsAndChilds(states, proccesed_states, global_state)
        return StateMachine(
            states=[global_state, *list(proccesed_states.values())],
            notes=notes,
            start_action='',
            signals=[*player_signals.keys(),
                     *user_signals,
                     *CJsonParser.addSignals(components, player_signals)],
            start_node=startNode
        )

    
    def getFiles(self, json_data):
        files = []

        for data in json_data:
            files.append(SourceFile(
                data['filename'], data['extension'], data['fileContent']))

        return files
