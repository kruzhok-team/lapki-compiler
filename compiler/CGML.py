"""Module for work with CyberiadaMl."""
import re
import random
from typing import Dict, List, Set, Any
from copy import deepcopy
from string import Template

from compiler.PlatformManager import PlatformManager
from compiler.types.ide_types import Bounds
from compiler.types.inner_types import InnerComponent, InnerEvent, InnerTrigger
from compiler.types.platform_types import (
    ClassParameter,
    Component,
    Platform,
    Signal
)
from cyberiadaml_py.cyberiadaml_parser import CGMLParser
from cyberiadaml_py.types.elements import (
    CGMLElements,
    CGMLState,
    CGMLTransition,
    CGMLComponent,
    CGMLInitialState
)
from compiler.fullgraphmlparser.stateclasses import (
    StateMachine,
    ParserState,
    ParserTrigger,
    ParserNote,
    Labels,
    create_note,
    SMCompilingSettings
)

_TransitionId = str
_StateId = str
_ComponentId = str

_INCLUDE_TEMPLATE = Template('#include "$component_type"')
_CALL_FUNCTION_TEMPLATE = Template('$id$delimeter$method($args);')
_CHECK_SIGNAL_TEMPLATE = Template(
    """
if($condition) {
    SIMPLE_DISPATCH(the_sketch, $signal);
}
"""
)


class CGMLException(Exception):
    """Errors occured during CGML processing."""

    ...


def __parse_trigger(trigger: str, regexes: List[str]) -> InnerTrigger:
    """Get condition and trigger by regexes."""
    if trigger is None or trigger == '':
        raise CGMLException('Trigger is None!')
    for regex in regexes:
        regex_match = re.match(regex, trigger)
        if regex_match is None:
            continue

        parsed_trigger: str = regex_match.group('trigger')

        regex_dict = regex_match.groupdict()
        condition = regex_dict.get('condition', None)
        postfix = regex_dict.get('postfix', None)
        return InnerTrigger(parsed_trigger, condition, postfix)
    raise CGMLException(f'Trigger({trigger}) doesnt match any regex!')


def __parse_actions(actions: str) -> List[InnerEvent]:
    """Parse action field of CGMLElements and returns do,\
        triggers, conditions."""
    events: List[InnerEvent] = []
    raw_events = actions.split('\n\n')

    for raw_event in raw_events:
        raw_trigger, do = raw_event.split('/')
        inner_trigger = __parse_trigger(
            raw_trigger,
            [
                (r'^(?P<trigger>[^\[\]]+)\[(?P<condition>.+)\] '
                 r'(?P<postfix>w+)$'),
                r'^(?P<trigger>[^\[\]]+) (?P<postfix>.+)$',
                r'^(?P<trigger>[^\[\]]+)\[(?P<condition>.+)\]$',
                r'^(?P<trigger>[^\[\]]+)$'
            ]
        )
        check_function: str | None = None
        if '.' in inner_trigger.trigger:
            check_function = inner_trigger.trigger
            inner_trigger.trigger = inner_trigger.trigger.replace('.', '_')
        events.append(InnerEvent(
            inner_trigger,
            do,
            check_function,
        ))
    return events


def __create_empty_bounds() -> Bounds:
    return Bounds(
        x=0,
        y=0,
        height=0,
        width=0
    )


def __gen_id() -> int:
    return random.randint(0, 100)


def __process_state(state_id: str,
                    cgml_state: CGMLState,
                    default_propagate: bool = False) -> ParserState:
    """Process internal triggers and actions of state."""
    inner_triggers: List[InnerEvent] = __parse_actions(cgml_state.actions)
    parser_triggers: List[ParserTrigger] = []
    entry = ''
    exit = ''
    for inner in inner_triggers:
        trigger = inner.event.trigger
        match trigger:
            case 'entry':
                entry = inner.actions
            case 'exit':
                exit = inner.actions
            case _:
                condition = inner.event.condition
                propagate = default_propagate
                if inner.event.postfix is not None:
                    match inner.event.postfix:
                        case 'propagate':
                            propagate = True
                        case 'block':
                            propagate = False
                        case '_':
                            raise CGMLException(
                                f'Неизвестный постфикс {inner.event.postfix} '
                                'допустимые значения "propagate", "block"'
                            )
                parser_triggers.append(
                    ParserTrigger(
                        id=str(__gen_id()),
                        name=inner.event.trigger,
                        source=state_id,
                        target='',
                        type='internal',
                        action=inner.actions,
                        defer=inner.actions.strip() == 'defer',
                        guard=(condition
                               if condition is not None else 'true'),
                        check_function=inner.check,
                        propagate=propagate
                    )
                )
    bounds = (
        cgml_state.bounds
        if cgml_state.bounds is not None
        else __create_empty_bounds()
    )
    return ParserState(
        id=state_id,
        new_id=[state_id],
        name=cgml_state.name,
        type='internal',
        entry=entry,
        exit=exit,
        parent=None,
        bounds=bounds,
        actions='',
        trigs=parser_triggers,
        childs=[]
    )


def __process_transition(
        transition_id: str,
        cgml_transition: CGMLTransition) -> ParserTrigger:
    """Parse CGMLTransition and convert to ParserTrigger\
        - class for fullgraphmlparser."""
    inner_triggers: List[InnerEvent] = __parse_actions(cgml_transition.actions)

    if len(inner_triggers) == 0:
        raise Exception('No trigger for transition!')
    # TODO: Обработка нескольких событий для триггера
    inner_event: InnerEvent = inner_triggers[0]
    inner_trigger: InnerTrigger = inner_event.event
    inner_trigger.trigger = inner_trigger.trigger.replace('.', '_')
    condition = (
        inner_trigger.condition
        if inner_trigger.condition is not None
        else 'true')
    return ParserTrigger(
        name=inner_trigger.trigger,
        source=cgml_transition.source,
        target=cgml_transition.target,
        action=inner_event.actions,
        id=transition_id,
        type='external',
        guard=condition,
        check_function=inner_event.check
    )


def __connect_transitions_to_states(
    states: Dict[_StateId, ParserState],
    transitions: List[ParserTrigger]
) -> Dict[_StateId, ParserState]:
    """Add external triggers to states."""
    states_with_external_trigs = deepcopy(states)
    for transition in transitions:
        source_state = states_with_external_trigs.get(transition.source)
        if source_state is None:
            raise CGMLException('Source state is None!')
        source_state.trigs.append(transition)

    return states_with_external_trigs


def __connect_parents_to_states(
    parser_states: Dict[_StateId, ParserState],
    cgml_states: Dict[_StateId, CGMLState],
    global_state: ParserState
) -> Dict[_StateId, ParserState]:
    """
    Fill parent field for states.

    We can't fill it during first iteration,\
        because not everyone state is ready.
    So we can't add parent, that doesn't exist yet.
    """
    states_with_parents = deepcopy(parser_states)

    for state_id in cgml_states:
        cgml_state = cgml_states[state_id]
        parser_state = states_with_parents[state_id]
        parent = cgml_state.parent
        if parent is None:
            parser_state.parent = global_state
            global_state.childs.append(parser_state)
        else:
            parent_state = states_with_parents[parent]
            parser_state.parent = parent_state
            parent_state.childs.append(parser_state)
            parent_state.type = 'group'

    return states_with_parents


def __get_all_triggers(states: List[ParserState],
                       transitions: List[ParserTrigger]
                       ) -> List[ParserTrigger]:
    """Get all triggers from states and transitions."""
    triggers: List[ParserTrigger] = []
    for state in states:
        triggers.extend(state.trigs)
    triggers.extend(transitions)
    return triggers


def __get_signals_set(
        triggers: List[ParserTrigger]) -> Set[str]:
    """Get signals set from triggers."""
    signals = set()
    for trig in triggers:
        signals.add(trig.name)

    return signals


def __parse_components(
    components: List[CGMLComponent]
) -> Dict[_ComponentId, InnerComponent]:
    """Parse component's parameters."""
    inner_components: Dict[_ComponentId, InnerComponent] = {}

    for component in components:
        parameters: List[str] = component.parameters.split('\n')
        type = ''
        parsed_parameters: Dict[str, str] = {}
        for parameter in parameters:
            parameter_name, value = list(map(
                lambda val: val.strip(), parameter.split('/')))
            if parameter_name is None or value is None:
                raise CGMLException(
                    'No name parameter or no value of parameter.')
            match parameter_name:
                case 'type':
                    type = value
                case 'labelColor' | 'label' | 'name' | 'description':
                    ...
                case _:
                    parsed_parameters[parameter_name] = value
        inner_components[component.id] = InnerComponent(
            type,
            parsed_parameters
        )

    return inner_components


def __generate_create_components_code(
        components: Dict[_ComponentId, InnerComponent],
        platform: Platform) -> List[ParserNote]:
    """
    Generate code, that create component's variables in h-file.

    Generated code example:
    ```cpp
    LED led1 = LED(12);
    Timer timer1 = Timer();
    ```
    """
    notes: List[ParserNote] = []
    for component_id in components:
        component: InnerComponent = components[component_id]
        type: str = component.type
        platform_component = platform.components[type]

        if platform_component.singletone:
            continue

        construct_parameters = platform_component.constructorParameters
        args: str = __create_parameters_sequence(
            component.parameters, construct_parameters)
        code_to_insert = (f'{type} {component_id} = '
                          f'{type}({args});\n')
        notes.append(create_note(Labels.H, code_to_insert))
    return notes


def __create_parameters_sequence(
        component_parameters: Dict[str, Any],
        platform_parameters: Dict[str, ClassParameter]) -> str:
    """
    Create args sequence from component's parameters and platform parameters.

    Order of parameters in sequence depends on order platform parameters.
    Return example: 'arg1, arg2, arg3'
    """
    args: List[str] = []
    for parameter_name in platform_parameters:
        parameter = component_parameters[parameter_name]
        if parameter is None:
            if platform_parameters[parameter_name].optional:
                continue
            else:
                raise CGMLException(
                    f'No arg {parameter_name} for component!')
        args.append(str(parameter))
    return ', '.join(args)


def __generate_function_call(
        platform: Platform,
        component_type: str,
        component_id: str,
        method: str,
        args: str) -> str:
    """
    Generate function call code using _CALL_FUNCTION_TEMPLATE.

    Check component's static and platform's static.
    """
    delimeter = '.'
    if (platform.staticComponents or
            platform.components[component_type].singletone):
        delimeter = '::'
    return _CALL_FUNCTION_TEMPLATE.substitute(
        {
            'id': component_id,
            'method': method,
            'args': args,
            'delimeter': delimeter
        }
    )


def __generate_signal_checker(
        platform: Platform,
        component_type: str,
        component_id: str,
        method: str,
        signal_name: str
) -> str:
    """Generate code part for checking and emitting signals using\
        _CHECK_SIGNAL_TEMPLATE."""
    platform_component: Component = platform.components[component_type]
    signal: Signal = platform_component.signals[method]
    call_method = signal.checkMethod
    condition = __generate_function_call(
        platform, component_type, component_id, call_method, '')
    return _CHECK_SIGNAL_TEMPLATE.substitute({
        'signal': signal_name,
        'condition': condition.rstrip(';')
    })


def __generate_loop_signal_checks_code(
    platform: Platform,
    triggers: List[ParserTrigger],
    components: Dict[_ComponentId, InnerComponent]
) -> List[ParserNote]:
    """
    Generate code for checking signals in loop function.

    Generated code example:
    ```cpp
    if(timer1.timeout()) {
        SIMPLE_DISPATCH(the_sketch, timer1_timeout);
    }
    ```
    """
    checked_signals: Set[str] = set()
    notes: List[ParserNote] = []

    for trigger in triggers:
        if trigger.name in checked_signals:
            continue
        # Предполагается, что в check_function лежит строка вида
        # component.method
        condition = trigger.check_function
        if condition is None:
            continue
        component_id, method = condition.split('.')
        type = components[component_id].type
        code_to_insert = __generate_signal_checker(
            platform, type, component_id, method, trigger.name)
        notes.append(create_note(Labels.LOOP, code_to_insert))
        checked_signals.add(trigger.name)
    return notes


def __generate_loop_tick_actions_code(
    platform: Platform,
    components: Dict[_ComponentId, InnerComponent]
) -> List[ParserNote]:
    """
    Generate code, that will be call every loop's tick.

    Generated code example:
    ```cpp
    analogIn.read();
    ```
    """
    notes: List[ParserNote] = []
    for component_id, component in components.items():
        platform_component: Component = platform.components[component.type]
        loop_actions: List[str] = platform_component.loopActions
        for method in loop_actions:
            code_to_insert = __generate_function_call(
                platform, component.type, component_id, method, '')
            notes.append(create_note(Labels.LOOP, code_to_insert))
    return notes


def __generate_setup_function_code(
        components: Dict[_ComponentId, InnerComponent],
        platform: Platform) -> List[ParserNote]:
    """
    Generate code for initialization components in setup function.

    Base of setup function generates in CppWriter!

    Generated code example:
    ```cpp
    QHsmSerial::init(9600);
    ```
    """
    notes: List[ParserNote] = []
    for component_id, component in components.items():
        type = component.type
        platform_component = platform.components[type]
        init_func = platform_component.initializationFunction
        if init_func is None:
            continue
        init_parameters = platform_component.initializationParameters
        args = ''
        if init_parameters is not None:
            args = __create_parameters_sequence(
                component.parameters, init_parameters)
        code_to_insert = __generate_function_call(
            platform, type, component_id, init_func, args)
        notes.append(create_note(Labels.SETUP, code_to_insert))
    return notes


def __get_include_libraries(platform: Platform,
                            components: List[InnerComponent]) -> Set[str]:
    """Get set of source files, that must be included."""
    included_libraries: Set[str] = set()
    for component in components:
        included_libraries.update(
            platform.components[component.type].importFiles)
    return included_libraries


def __generate_includes_libraries_code(
    included_libraries: Set[str]
) -> List[ParserNote]:
    """Generate code for including libraries using _INCLUDE_TEMPLATE."""
    return [create_note(
        Labels.H_INCLUDE,
        _INCLUDE_TEMPLATE.substitute({'component_type': type})
    ) for type in included_libraries]


def __get_build_files(
    platform: Platform,
    components: List[InnerComponent]
) -> Set[str]:
    """Get set of files, that must be included for compiling."""
    build_libraries: Set[str] = set()
    for component in components:
        build_libraries.update(
            platform.components[component.type].buildFiles)
    return build_libraries


def __add_initial_states(
    initial_states: Dict[str, CGMLInitialState],
    states: Dict[str, ParserState]
) -> Dict[str, ParserState]:
    for initial_id, initial in initial_states.items():
        # initial.


async def parse(xml: str) -> StateMachine:
    """
    Parse XML with cyberiadaml-py library and convert it\
        to StateMachine for CppWriter class.

    Generate code:
    - creating component's variables;
    - initialization components in setup function;
    - signal checks in loop function;
    """
    parser = CGMLParser()
    cgml_scheme: CGMLElements = parser.parseCGML(xml)
    platfrom_manager = PlatformManager()
    platform: Platform = await platfrom_manager.get_platform(
        cgml_scheme.platform, '')  # TODO: Доставать версию платформы
    if not platform.compile or platform.compilingSettings is None:
        raise CGMLException(
            f'Platform {platform.name} not supporting compiling!')
    global_state = ParserState(
        name='global',
        type='group',
        actions='',
        trigs=[],
        entry='',
        exit='',
        id='global',
        new_id=['global'],
        parent=None,
        childs=[],
        bounds=Bounds(
            x=0,
            y=0,
            height=0,
            width=0
        )
    )
    # Parsing external transitions.
    transitions: List[ParserTrigger] = []
    cgml_transitions: Dict[_TransitionId,
                           CGMLTransition] = cgml_scheme.transitions
    for transition_id in cgml_transitions:
        cgml_transition = cgml_transitions[transition_id]
        transitions.append(__process_transition(
            transition_id, cgml_transition))

    # Parsing state's actions, internal triggers, entry/exit
    states: Dict[_StateId, ParserState] = {}
    cgml_states: Dict[_StateId, CGMLState] = cgml_scheme.states
    for state_id in cgml_states:
        cgml_state = cgml_states[state_id]
        states[state_id] = __process_state(state_id, cgml_state)

    states_with_transitions = __connect_transitions_to_states(
        states,
        transitions
    )
    states_with_parents = __connect_parents_to_states(
        states_with_transitions, cgml_states, global_state)
    # TODO: Добавить внешние переходы в triggers
    if cgml_scheme.initial_state is None:
        raise CGMLException('No initial state!')

    start_node: str = cgml_scheme.initial_state.target
    # Мы получаем список триггеров для того, чтобы потом:
    # 1) Сформировать набор всех сигналов
    # 2) Сгенерировать проверки для вызова сигналов
    all_triggers = __get_all_triggers(
        list(states_with_parents.values()),
        transitions)
    signals = __get_signals_set(all_triggers)

    parsed_components = __parse_components(cgml_scheme.components)
    included_libraries: Set[str] = __get_include_libraries(
        platform, list(parsed_components.values()))
    build_files = __get_build_files(platform, list(parsed_components.values()))
    notes: List[ParserNote] = [
        *__generate_create_components_code(parsed_components, platform),
        *__generate_setup_function_code(parsed_components, platform),
        *__generate_includes_libraries_code(included_libraries),
        * __generate_loop_tick_actions_code(platform, parsed_components),
        *__generate_loop_signal_checks_code(platform,
                                            all_triggers,
                                            parsed_components)
    ]
    compiling_settings = SMCompilingSettings(
        included_libraries,
        build_files,
        platform.id,
        platform.compilingSettings
    )
    return StateMachine(
        start_node=start_node,
        name='sketch',
        start_action='',
        notes=notes,
        states=[global_state, *list(states_with_parents.values())],
        signals=signals,
        compiling_settings=compiling_settings
    )
