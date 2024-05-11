import os.path
import inspect
from string import Template

import re
from collections import defaultdict
from typing import List, Tuple, Dict
from aiofile import async_open
try:
    from .stateclasses import ParserState, ParserTrigger, StateMachine, Labels
    from .graphml import *
except ImportError:
    from compiler.fullgraphmlparser.stateclasses import (
        ParserState,
        ParserTrigger,
        StateMachine,
        Labels
    )
    from compiler.fullgraphmlparser.graphml import *

MODULE_PATH = os.path.dirname(os.path.abspath(inspect.stack()[0][1]))

INITIAL_STATE_TEMPLATE = Template('''\
QState $id() {
    

}    
'''
                                  )


def get_enum(text_labels: List[str]) -> str:
    """
    prepares list of signals for enum structure for c language: joins them into one string comma and \n-separated
    and adds _SIG to each signal
     Example:
        >>> get_enum(["EVENT1", "EVENT2"])
        "EVENT1_SIG,
         EVENT2_SIG"
    :param text_labels:
    :return: string
    """
    enum_labels: List[str] = [label + '_SIG' for label in text_labels]
    enum_labels.append('\n\nLAST_USER_SIG\n};')
    enum = ',\n'.join(enum_labels)
    enum = 'enum PlayerSignals {\nTICK_SEC_SIG = Q_USER_SIG,\n\n' + enum
    return enum


class CppFileWriter:
    id_to_name = {}
    notes_dict = {}
    f = None
    all_signals = []
    userFlag = False  # Флаг на наличие кода для класса User
    list_notes_dict: Dict[str, List[str]] = {}

    def __init__(self,
                 state_machine: StateMachine,
                 create_setup=False,
                 create_loop=False) -> None:
        self.create_loop = create_loop
        self.create_setup = create_setup
        self.sm_name = state_machine.name
        self.player_signal = state_machine.signals
        notes_mapping: List[Tuple[str, str]] = [
            (Labels.H_INCLUDE.value, 'raw_h_code'),
            (Labels.H.value,
             'declare_h_code'),
            (Labels.CPP.value,
             'raw_cpp_code'),
            (Labels.CTOR_FIELDS.value,
             'constructor_fields'),
            (Labels.STATE_FIELDS.value, 'state_fields'),
            (Labels.CTOR.value,
             'constructor_code'),
            (Labels.EVENT_FIELDS.value, 'event_fields'),
            (Labels.USER_VAR_H.value,
             'user_variables_h'),
            (Labels.USER_FUNC_H.value,
             'user_methods_h'),
            (Labels.USER_VAR_C.value,
             'user_variables_c'),
            (Labels.USER_FUNC_C.value, 'user_methods_c'),
            (Labels.SETUP.value, 'setup'),
            (Labels.LOOP.value, 'loop')
        ]

        self.list_notes_dict: Dict[str, List[str]] = {key: [''] for _, key
                                                      in notes_mapping}

        for note in state_machine.notes:
            dict_note = note.model_dump(by_alias=True)
            for prefix, key in notes_mapping:
                # Делаем так, чтобы при повторении меток, их содержимое
                # объединялось, а не перезаписывалось
                if dict_note['y:UMLNoteNode']['y:NodeLabel']['#text'].startswith(prefix):
                    if self.list_notes_dict != []:
                        self.list_notes_dict[key].append(
                            '\n'.join(dict_note['y:UMLNoteNode']['y:NodeLabel']['#text'].split('\n')[1:]))
                    else:
                        self.list_notes_dict[key].append(
                            dict_note['y:UMLNoteNode']['y:NodeLabel']['#text'])
        self.notes_dict = {key: '\n'.join(
            self.list_notes_dict[key]) for _, key in notes_mapping}

        self.start_node = state_machine.start_node
        self.start_action = state_machine.start_action
        self.states = state_machine.states
        for state in self.states:
            self.id_to_name[state.id] = state.name
            for trigger in state.trigs:
                if trigger.guard:
                    trigger.guard = trigger.guard.strip()
        self.initial_states = state_machine.initial_states

    async def _write_initial_states(self):
        for initial in self.initial_states:
            await self._write_full_line_comment(
                f'Initial pseudostate {initial.id}', ' ')
            await self._insert_string()

    async def write_to_file(self, folder: str, extension: str):
        async with async_open(os.path.join(folder, f'{self.sm_name}.{extension}'), 'w') as f:
            self.f = f
            await self._insert_file_template('preamble_c.txt')
            await self._write_constructor()
            await self._write_initial()
            await self._write_states_definitions_recursively(self.states[0], 'SMs::%s::SM' % self._sm_capitalized_name())
            await self._write_states_definitions_recursively
            await self._insert_file_template('footer_c.txt')
            if self.notes_dict['setup'] or self.create_setup:
                await self._insert_string('\nvoid setup() {')
                await self._insert_string('\n\tSketch_ctor();')
                await self._insert_string('\n\tQEvt event;')
                await self._insert_string('\n\tQMsm_init(the_sketch, &event);')
                await self._insert_string('\n\t' + '\n\t'.join(self.notes_dict['setup'].split('\n')[1:]))
                await self._insert_string('\n}')
            if self.notes_dict['loop'] or self.create_loop:
                await self._insert_string('\nvoid loop() {')
                await self._insert_file_template('defer_loop.txt')
                await self._insert_string('\n\t' + '\n\t'.join(self.notes_dict['loop'].split('\n')[1:]))
                await self._insert_string('\n}')
            if self.notes_dict['raw_cpp_code']:
                await self._insert_string('\n//Start of c code from diagram\n')
                await self._insert_string('\n'.join(self.notes_dict['raw_cpp_code'].split('\n')[1:]) + '\n')
                await self._insert_string('//End of c code from diagram\n\n\n')
            self.f = None
        async with async_open(os.path.join(folder, 'User.h'), "w") as f:
            self.f = f
            await self._insert_file_template('user_preamble_h.txt')

            if self.notes_dict['user_variables_h']:
                await self._insert_string('\n'.join(self.notes_dict['user_variables_h'].split('\n')[1:]) + '\n')
                self.userFlag = True
            if self.notes_dict['user_methods_c']:
                await self._insert_string('\n'.join(self.notes_dict['user_methods_h'].split('\n')[1:]) + '\n')
                self.userFlag = True
            await self._insert_file_template('user_footer_h.txt')

        async with async_open(os.path.join(folder, f'User.{extension}'), "w") as f:
            self.f = f

            if self.notes_dict['user_variables_h']:
                await self._insert_string('\n'.join(self.notes_dict['user_variables_h'].split('\n')[1:]) + '\n')

            if self.notes_dict['user_methods_h']:
                await self._insert_string('\n'.join(self.notes_dict['user_methods_h'].split('\n')[1:]) + '\n')

        async with async_open(os.path.join(folder, f'User.{extension}'), "w") as f:
            self.f = f
            await self._insert_file_template('user_preamble_c.txt')

            await self._insert_string('// Start variables\n')
            if self.notes_dict['user_variables_c']:
                await self._insert_string('\n'.join(self.notes_dict['user_variables_c'].split('\n')[1:]) + '\n')
            await self._insert_string('// end variables\n')

            if self.notes_dict['user_methods_c']:
                await self._insert_string('\n'.join(self.notes_dict['user_methods_c'].split('\n')[1:]) + '\n')

        async with async_open(os.path.join(folder, '%s.h' % self.sm_name), 'w') as f:
            self.f = f

            await self._insert_file_template('preamble_h.txt')
            if self.userFlag:
                await self._insert_string('#include "User.h"\n')
            if self.notes_dict['raw_h_code']:
                await self._insert_string('//Start of h code from diagram\n')
                await self._insert_string('\n'.join(self.notes_dict['raw_h_code'].split('\n')[1:]) + '\n')
                await self._insert_string('//End of h code from diagram\n\n\n')

            await self._write_full_line_comment('.$declare${SMs::STATE_MACHINE_CAPITALIZED_NAME}', 'v')
            await self._write_full_line_comment('.${SMs::STATE_MACHINE_CAPITALIZED_NAME}', '.')
            await self._insert_string('typedef struct {\n')
            await self._insert_string('/* protected: */\n')
            await self._insert_string('    QHsm super;\n')
            await self._insert_string('\n')
            await self._insert_string('/* public: */\n')
            constructor_fields: str = self.notes_dict['state_fields']
            await self._insert_string('    ' + '\n    '.join(constructor_fields.split('\n')[1:]) + '\n')
            await self._insert_string('} STATE_MACHINE_CAPITALIZED_NAME;\n\n')
            await self._insert_string('/* protected: */\n')
            await self._insert_string('QState STATE_MACHINE_CAPITALIZED_NAME_initial(STATE_MACHINE_CAPITALIZED_NAME * const me, void const * const par);\n')
            await self._write_states_declarations_recursively(self.states[0])
            await self._insert_string('\n#ifdef DESKTOP\n')
            await self._insert_string(
                'QState STATE_MACHINE_CAPITALIZED_NAME_final(STATE_MACHINE_CAPITALIZED_NAME * const me, QEvt const * const e);\n')
            await self._insert_string('#endif /* def DESKTOP */\n\n')
            await self._write_full_line_comment('.$enddecl${SMs::STATE_MACHINE_CAPITALIZED_NAME}', '^')
            await self._insert_string('extern QHsm * const the_STATE_MACHINE_NAME; /* opaque pointer to the STATE_MACHINE_NAME HSM */\n\n')

            await self._insert_string('typedef struct STATE_MACHINE_NAMEQEvt {\n')
            await self._insert_string('    QEvt super;\n')
            event_fields: str = self.notes_dict['event_fields']
            await self._insert_string('    ' + '\n    '.join(event_fields.split('\n')[1:]) + '\n')
            await self._insert_string('} STATE_MACHINE_NAMEQEvt;\n\n')
            await self._insert_string(get_enum(self.player_signal) + '\n')
            await self._insert_string('\nstatic STATE_MACHINE_CAPITALIZED_NAME STATE_MACHINE_NAME; /* the only instance of the STATE_MACHINE_CAPITALIZED_NAME class */\n\n\n\n')
            await self._write_full_line_comment('.$declare${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', 'v')
            await self._write_full_line_comment('.${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', '.')
            await self._insert_string('void STATE_MACHINE_CAPITALIZED_NAME_ctor(')
            constructor_fields: str = self.notes_dict['constructor_fields']
            if constructor_fields:
                await self._insert_string(
                    '\n    ' + ',\n    '.join(constructor_fields.replace(';', '').split('\n')[1:]) + ');\n')
            else:
                await self._insert_string('void);\n')
            await self._write_full_line_comment('.$enddecl${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', '^')
            if self.notes_dict['declare_h_code']:
                await self._insert_string('//Start of h code from diagram\n')
                await self._insert_string('\n'.join(self.notes_dict['declare_h_code'].split('\n')[1:]) + '\n')
                await self._insert_string('//End of h code from diagram\n\n\n')
            await self._insert_file_template('footer_h.txt')
            self.f = None

    async def _write_constructor(self):
        await self._write_full_line_comment('.$define${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', 'v')
        await self._write_full_line_comment('.${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', '.')
        await self._insert_string('void STATE_MACHINE_CAPITALIZED_NAME_ctor(')
        constructor_fields: str = self.notes_dict['constructor_fields']
        if constructor_fields:
            await self._insert_string('\n    ' + ',\n    '.join(constructor_fields.replace(';', '').split('\n')[1:]) + ')\n')
            await self._insert_string('{\n')
        else:
            await self._insert_string('void) {\n')
        await self._insert_string('    STATE_MACHINE_CAPITALIZED_NAME *me = &STATE_MACHINE_NAME;\n')
        constructor_code: str = self.notes_dict['constructor_code']
        await self._insert_string('     ' + '\n    '.join(constructor_code.replace('\r', '').split('\n')[1:]))
        await self._insert_string('\n')
        await self._insert_string('    QHsm_ctor(&me->super, Q_STATE_CAST(&STATE_MACHINE_CAPITALIZED_NAME_initial));\n')
        await self._insert_string('}\n')
        await self._write_full_line_comment('.$enddef${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', '^')

    async def _write_initial(self):
        await self._write_full_line_comment('.$define${SMs::STATE_MACHINE_CAPITALIZED_NAME}', 'v')
        await self._write_full_line_comment('.${SMs::STATE_MACHINE_CAPITALIZED_NAME}', '.')
        await self._write_full_line_comment('.${SMs::STATE_MACHINE_CAPITALIZED_NAME::SM}', '.')
        await self._insert_string(
            'QState STATE_MACHINE_CAPITALIZED_NAME_initial(STATE_MACHINE_CAPITALIZED_NAME * const me, void const * const par) {\n')
        await self._insert_string('    /*.${SMs::STATE_MACHINE_CAPITALIZED_NAME::SM::initial} */\n')
        await self._insert_string('    %s\n' % self.start_action)
        await self._insert_string(
            '    return Q_TRAN(&STATE_MACHINE_CAPITALIZED_NAME_%s);\n' % self.start_node)
        await self._insert_string('}\n')

    async def _write_guard_comment(self, f, state_path: str, event_name: str, guard: str):
        prefix = '            /*.${%s::%s::[' % (state_path, event_name)
        suffix = ']} */\n'
        guard_tokens = guard.replace('+', ' ').split(' ')
        shortened_guard = guard_tokens[0]
        i_token = 1
        while i_token < len(guard_tokens) and len(prefix) + len(shortened_guard) + len(guard_tokens[i_token]) + len(suffix) <= 121:
            shortened_guard = shortened_guard + guard_tokens[i_token]
            i_token = i_token + 1
        if i_token != len(guard_tokens):
            suffix = '~' + suffix[1:]

        return await self._insert_string(prefix + shortened_guard + suffix)

    async def _write_full_line_comment(self, text: str, filler: str):
        await self._insert_string(('/*' + text.replace('STATE_MACHINE_NAME', self.sm_name).replace('STATE_MACHINE_CAPITALIZED_NAME', self._sm_capitalized_name()) + ' ').ljust(76, filler) + '*/\n')

    def _sm_capitalized_name(self) -> str:
        return self.sm_name[0].upper() + self.sm_name[1:]

    async def _insert_string(self, s: str):
        await self.f.write(re.sub('[ ]*\n', '\n',
                                  s.replace('STATE_MACHINE_NAME', self.sm_name).replace('STATE_MACHINE_CAPITALIZED_NAME', self._sm_capitalized_name())))

    async def _insert_file_template(self, filename: str):
        async with async_open(os.path.join(MODULE_PATH, 'templates', filename)) as input_file:
            async for line in input_file:
                await self._insert_string(str(line))

    async def _insert_defer(self, trigger_name: str) -> None:
        await self._insert_string('                if (!signalDefer) {')
        await self._insert_string(f'                                defer[defer_i] = {trigger_name}_SIG;\n')
        await self._insert_string('                                defer_i++;\n')
        await self._insert_string('                }')

    async def _write_states_definitions_recursively(self, state: ParserState, state_path: str):
        state_path = state_path + '::' + state.name
        state_comment = '/*.${' + state_path + '} '
        state_comment = state_comment + '.' * \
            (76 - len(state_comment)) + '*/\n'
        await self.f.write(state_comment)
        await self._insert_string('QState STATE_MACHINE_CAPITALIZED_NAME_%s(STATE_MACHINE_CAPITALIZED_NAME * const me, QEvt const * const e) {\n' % state.id)
        await self._insert_string('    QState status_;\n')
        await self._insert_string('    switch (e->sig) {\n')

        if state.name == 'global':
            await self._insert_file_template('terminate_sig_c.txt')
        else:
            await self._insert_string('        /*.${' + state_path + '} */\n')
            await self._insert_string('        case Q_ENTRY_SIG: {\n')
            await self._insert_string('        \tstateChanged = false;\n')
            await self._insert_string('\n'.join(['            ' + line for line in state.entry.split('\n')]) + '\n')
            await self._insert_string('            status_ = Q_HANDLED();\n')
            await self._insert_string('            break;\n')
            await self._insert_string('        }\n')

            await self._insert_string('        /*.${' + state_path + '} */\n')
            await self._insert_string('        case Q_EXIT_SIG: {\n')
            await self._insert_string('\n'.join(['            ' + line for line in state.exit.split('\n')]) + '\n')
            await self._insert_string('            status_ = Q_HANDLED();\n')
            await self._insert_string('            break;\n')
            await self._insert_string('        }\n')

        name_to_triggers = defaultdict(list)
        name_to_position = {}

        for i, trigger in enumerate(state.trigs):
            if '?def' in trigger.name:
                continue

            name_to_triggers[trigger.name].append(trigger)
            name_to_position[trigger.name] = i

        triggers_merged: List[Tuple[str, List[ParserTrigger]]] = sorted(
            [(name, name_to_triggers[name]) for name in name_to_triggers],
            key=lambda t: name_to_position[t[0]])

        for event_name, triggers in triggers_merged:
            await self._insert_string('        /*.${%s::%s} */\n' % (state_path, event_name))
            await self._insert_string('        case %s_SIG: {\n' % event_name)
            if len(triggers) == 1:
                if triggers[0].guard:
                    await self._write_guard_comment(self.f, state_path, event_name, triggers[0].guard)
                    await self._insert_string('            if (%s) {\n' % triggers[0].guard)
                    await self._write_trigger(self.f, triggers[0], state_path, event_name, state, '    ')
                    await self._insert_string('            }\n')
                    await self._insert_string('            else {\n')
                    await self._insert_string('                status_ = Q_UNHANDLED();\n')
                    await self._insert_string('            }\n')
                else:
                    await self._write_trigger(self.f, triggers[0], state_path, event_name, state=state)
            elif len(triggers) == 2:
                if triggers[0].guard == 'else':
                    triggers[0], triggers[1] = triggers[1], triggers[0]
                await self._write_guard_comment(self.f, state_path, event_name, triggers[0].guard)
                await self._insert_string('            if (%s) {\n' % triggers[0].guard)
                await self._write_trigger(self.f, triggers[0], state_path, event_name, state, '    ')
                await self._insert_string('            }\n')
                await self._write_guard_comment(self.f, state_path, event_name, triggers[1].guard)
                await self._insert_string('            else {\n')
                await self._write_trigger(self.f, triggers[1], state_path, event_name, state, '    ')
                await self._insert_string('            }\n')
            else:
                raise Exception('"else if" guards are not supported')
            await self._insert_string('            break;\n')
            await self._insert_string('        }\n')

        await self._insert_string('        default: {\n')
        if state.parent:
            await self._insert_string('            status_ = Q_SUPER(&STATE_MACHINE_CAPITALIZED_NAME_%s);\n' % state.parent.id)
        else:
            await self._insert_string('            status_ = Q_SUPER(&QHsm_top);\n')
        await self._insert_string('            break;\n')
        await self._insert_string('        }\n')
        await self._insert_string('    }\n')
        await self._insert_string('    return status_;\n')
        await self._insert_string('}\n')

        for child_state in state.childs:
            await self._write_states_definitions_recursively(child_state, state_path)

        for trigger in state.trigs:
            if '?def' in trigger.name:
                continue
            if trigger.name not in self.all_signals:
                self.all_signals.append(trigger.name)

    async def _write_states_declarations_recursively(self, state: ParserState):
        await self._insert_string('QState STATE_MACHINE_CAPITALIZED_NAME_%s(STATE_MACHINE_CAPITALIZED_NAME * const me, QEvt const * const e);\n' % state.id)
        for child_state in state.childs:
            await self._write_states_declarations_recursively(child_state)

    async def _write_trigger(self, f, trigger: ParserTrigger, state_path: str, event_name: str, state: ParserState, offset=''):
        if trigger.defer:
            await self._insert_defer(trigger.name)
        elif trigger.action and not trigger.type == 'choice_start':
            await self._insert_string('\n'.join(
                [offset + '            ' + line for line in trigger.action.split('\n')]) + '\n')
        if trigger.type == 'internal':
            if trigger.propagate:
                if state.parent:
                    await self._insert_string('            status_ = Q_SUPER(&STATE_MACHINE_CAPITALIZED_NAME_%s);\n' % state.parent.id)
                else:
                    await self._insert_string('            status_ = Q_SUPER(&QHsm_top);\n')
            else:
                await self._insert_string(offset + '            status_ = Q_HANDLED();\n')
        elif trigger.type == 'external' or trigger.type == 'choice_result':
            await self._insert_string(offset + '            stateChanged = true;\n')
            await self._insert_string(offset + '            status_ = Q_TRAN(&STATE_MACHINE_CAPITALIZED_NAME_%s);\n' % trigger.target)
        elif trigger.type == 'choice_start':
            target_choice_node = next((s for s in self.states if s.id ==
                                      trigger.target and s.type == 'choice'), None)
            assert target_choice_node
            assert len(target_choice_node.trigs) == 2
            triggers = target_choice_node.trigs
            if triggers[0].guard == 'else':
                triggers[0], triggers[1] = triggers[1], triggers[0]
            triggers[0].action = trigger.action + triggers[0].action
            triggers[1].action = trigger.action + triggers[1].action
            await self._write_guard_comment(self.f, state_path, event_name, triggers[0].guard)
            await self._insert_string(offset + '            if (%s) {\n' % triggers[0].guard)
            await self._write_trigger(self.f, triggers[0], state_path, event_name, state, offset + '    ')
            await self._insert_string(offset + '            }\n')
            await self._write_guard_comment(self.f, state_path, event_name, triggers[1].guard)
            await self._insert_string(offset + '            else {\n')
            await self._write_trigger(self.f, triggers[1], state_path, event_name, state, offset + '    ')
            await self._insert_string(offset + '            }\n')
        else:
            raise Exception('Unknown trigger type: %s' % trigger.type)
