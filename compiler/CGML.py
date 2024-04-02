"""Module for work with CyberiadaMl."""
import re
import random
from typing import Dict, List, Set
from copy import deepcopy

from compiler.PlatformManager import PlatformManager
from compiler.types.ide_types import Bounds
from compiler.types.platform_types import Platform
from compiler.types.inner_types import InnerComponent, InnerEvent, InnerTrigger
from cyberiadaml_py.cyberiadaml_parser import CGMLParser
from cyberiadaml_py.types.elements import (
    CGMLElements,
    CGMLState,
    CGMLTransition,
    CGMLComponent
)
from fullgraphmlparser.stateclasses import (
    StateMachine,
    ParserState,
    ParserTrigger,
    ParserNote,
    Labels,
    create_note
)

TransitionId = str
StateId = str
ComponentId = str


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
        try:
            condition: str | None = regex_match.group('condition')
        except IndexError:
            condition = None
        return InnerTrigger(parsed_trigger, condition)
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
                r'^(?P<trigger>\w+\.\w+)\[(?P<condition>.+)\]$',
                r'^(?P<trigger>\w+\.[\w()]+)$',
                r'^(?P<trigger>\w+_\w+)$',
                r'^(?P<trigger>\w+)$',
            ]
        )
        check_function: str | None = None
        if '.' in inner_trigger.trigger:
            check_function = inner_trigger.trigger
            inner_trigger.trigger = inner_trigger.trigger.replace('.', '_')

        events.append(InnerEvent(
            inner_trigger,
            do,
            check_function
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


def __process_state(state_id: str, cgml_state: CGMLState) -> ParserState:
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
                parser_triggers.append(
                    ParserTrigger(
                        id=str(__gen_id()),
                        name=inner.event.trigger,
                        source=state_id,
                        target='',
                        type='internal',
                        action=inner.actions,
                        guard=condition if condition is not None else 'true',
                        check_function=inner.check
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
    states: Dict[StateId, ParserState],
    transitions: List[ParserTrigger]
) -> Dict[StateId, ParserState]:
    """Add external triggers to states."""
    states_with_external_trigs = deepcopy(states)
    for transition in transitions:
        source_state = states_with_external_trigs.get(transition.source)
        if source_state is None:
            raise CGMLException('Source state is None!')
        source_state.trigs.append(transition)

    return states_with_external_trigs


def __connect_parents_to_states(
    parser_states: Dict[StateId, ParserState],
    cgml_states: Dict[StateId, CGMLState],
    global_state: ParserState
) -> Dict[StateId, ParserState]:
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

    return states_with_parents


def __get_signals_set(
        states: List[ParserState],
        transitions: List[ParserTrigger]) -> Set[str]:
    """Get signals set from all states and transitions."""
    signals = set()
    for state in states:
        for internal_trig in state.trigs:
            signals.add(internal_trig.name)

    for transition in transitions:
        signals.add(transition.name)

    return signals


def __parse_components(
    components: List[CGMLComponent]
) -> Dict[ComponentId, InnerComponent]:
    """Parse component's parameters."""
    inner_components: Dict[ComponentId, InnerComponent] = {}

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
        components: Dict[ComponentId, InnerComponent],
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
        # Порядок вставки аргументов определяется по позиции ключа в платформе
        construct_keys = list(construct_parameters.keys())
        args: List[str] = []
        for parameter_name in construct_keys:
            parameter = component.parameters[parameter_name]
            if parameter is None:
                if construct_parameters[parameter_name].optional:
                    continue
                else:
                    raise CGMLException(
                        f'No arg {parameter_name} for component\
                            {component_id}!')
            args.append(str(parameter))
        ', '.join(args)
        code_to_insert = f'{type} {component_id} = {type}({args});\n'
        notes.append(create_note(Labels.H, code_to_insert))
    return notes


def __generate_code(
        platform: Platform,
        components: List[CGMLComponent]) -> List[ParserNote]:
    return []


def parse(xml: str) -> StateMachine:
    parser = CGMLParser()
    cgml_scheme: CGMLElements = parser.parseCGML(xml)
    platform: Platform = PlatformManager.getPlatform(cgml_scheme.platform)
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
    cgml_transitions: Dict[TransitionId,
                           CGMLTransition] = cgml_scheme.transitions
    for transition_id in cgml_transitions:
        cgml_transition = cgml_transitions[transition_id]
        transitions.append(__process_transition(
            transition_id, cgml_transition))

    # Parsing state's actions, internal triggers, entry/exit
    states: Dict[StateId, ParserState] = {}
    cgml_states: Dict[StateId, CGMLState] = cgml_scheme.states
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
    signals = __get_signals_set(
        list(states_with_parents.values()),
        transitions
    )

    parsed_components = __parse_components(cgml_scheme.components)
    notes: List[ParserNote] = [
        *__generate_create_components_code(parsed_components, platform)
    ]
    print(notes)

    return StateMachine(
        start_node=start_node,
        name='sketch',
        start_action='',
        notes=notes,  # TODO: Сгенерировать вставки для кода.
        states=[global_state, *list(states_with_parents.values())],
        signals=signals
    )