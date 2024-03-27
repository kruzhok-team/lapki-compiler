"""Module for work with CyberiadaMl."""
import re
import random
from typing import Dict, List

from cyberiadaml_py.types.elements import CGMLElements, CGMLState, CGMLTransition
from cyberiadaml_py.cyberiadaml_parser import CGMLParser
from compiler.PlatformManager import PlatformManager
from compiler.types.ide_types import Bounds
from compiler.types.platform_types import Platform
from compiler.types.inner_types import InnerEvent, InnerTrigger
from fullgraphmlparser.stateclasses import StateMachine, ParserState, ParserTrigger


def __parse_trigger(trigger: str, regexes: List[str]) -> InnerTrigger:
    if trigger is None or trigger == '':
        raise Exception('Trigger is None!')
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
    raise Exception(f'Trigger({trigger}) doesnt match any regex!')


def __parse_actions(actions: str) -> List[InnerEvent]:
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
        inner_trigger.trigger = inner_trigger.trigger.replace('.', '_')
        events.append(InnerEvent(
            inner_trigger,
            do
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
                        id=__gen_id(),
                        name=inner.event.trigger,
                        source=state_id,
                        target='',
                        type='internal',
                        action=inner.actions,
                        guard=condition if condition is not None else 'true'
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
    inner_triggers: List[InnerEvent] = __parse_actions(cgml_transition.actions)

    if len(inner_triggers) == 0:
        raise Exception('No trigger for transition!')
    # TODO: Обработка нескольких условий для триггера
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
        id=__gen_id(),
        type='external',
        guard=condition
    )


def parse(xml: str) -> StateMachine:
    parser = CGMLParser()
    cgml_scheme: CGMLElements = parser.parseCGML(xml)
    platform: Platform = PlatformManager.getPlatform(cgml_scheme.platform)
    states: List[ParserState] = []
    transitions: List[ParserTrigger] = []

    cgml_transitions: Dict[str, CGMLTransition] = cgml_scheme.transitions
    for transition_id in cgml_transitions:
        cgml_transition = cgml_transitions[transition_id]
        transitions.append(__process_transition(
            transition_id, cgml_transition))

    cgml_states: Dict[str, CGMLState] = cgml_scheme.states
    for state_id in cgml_states:
        cgml_state = cgml_states[state_id]
        states.append(__process_state(state_id, cgml_state))
    # TODO: Добавить внешние переходы в triggers
    # sm: StateMachine = StateMachine()
