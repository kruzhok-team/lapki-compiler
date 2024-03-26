"""Module for work with CyberiadaMl."""
import re
from typing import List

from cyberiadaml_py.types.elements import CGMLElements, CGMLState
from cyberiadaml_py.cyberiadaml_parser import CGMLParser
from compiler.PlatformManager import PlatformManager
from compiler.types.platform_types import Platform
from compiler.types.inner_types import InnerEvent, InnerTrigger
from fullgraphmlparser.stateclasses import StateMachine, ParserState


def __parse_trigger(trigger: str) -> InnerTrigger:
    if trigger is None or trigger == '':
        raise Exception('Trigger is None!')

    with_condition = re.match(
        r'(?P<trigger>\w+\.\w+)\[(?P<condition>.+)\]', trigger)

    if with_condition is not None:
        condition: str = with_condition.group('condition')
        parsed_trigger: str = with_condition.group('trigger')
        return InnerTrigger(parsed_trigger, condition)

    right_trigger_without_condition = re.match(
        r'(?P<trigger>\w+\.\w+)', trigger)

    if right_trigger_without_condition is not None:
        parsed_trigger = right_trigger_without_condition.group('trigger')
        return InnerTrigger(parsed_trigger, None)

    raise Exception(f'Trigger({trigger}) doesnt match any regex!')


def __parse_actions(actions: str) -> List[InnerEvent]:
    events: List[InnerEvent] = []
    raw_events = actions.split('\n\n')

    for raw_event in raw_events:
        trigger, do = raw_event.split('/')
        if trigger is None:
            raise Exception('Trigger is None!')
    return events


def __process_state(cgml_state: CGMLState) -> ParserState:
    actions = __parse_actions(cgml_state.actions)

    return ParserState(
        name=cgml_state.name,
        type='internal')


def parse(xml: str) -> StateMachine:
    parser = CGMLParser()
    cgml_scheme: CGMLElements = parser.parseCGML(xml)
    platform: Platform = PlatformManager.getPlatform(cgml_scheme.platform)
    parsed_states: List[ParserState] = []
    cgml_states = cgml_scheme.states
    for state_id in cgml_states:
        cgml_state = cgml_states[state_id]
        parsed_state = __process_state(cgml_state)

    # sm: StateMachine = StateMachine()
