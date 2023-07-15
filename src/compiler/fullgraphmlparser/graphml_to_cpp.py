import os.path

import re
from collections import defaultdict
from graphml import *
from typing import List, Tuple
from stateclasses import State, Trigger

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
    enum = ',\n'.join(enum_labels)
    enum = 'enum PlayerSignals {\nTICK_SEC_SIG = Q_USER_SIG,\n\n' + enum
    enum = enum + ',\n\nLAST_USER_SIG\n};'
    return enum

class CppFileWriter:
    id_to_name = {}
    notes_dict = {}
    f = None
    all_signals = []

    def __init__(self, sm_name: str, start_node: str, start_action: str, states: List[State], notes: List[Dict[str, Any]], player_signal: List[str]):
        self.sm_name = sm_name
        self.player_signal = player_signal

        notes_mapping = [('Code for h-file', 'raw_h_code'),
                         ('Code for cpp-file', 'raw_cpp_code'),
         ('Constructor fields', 'constructor_fields'),
         ('State fields', 'state_fields'),
         ('Constructor code', 'constructor_code'),
         ('Event fields', 'event_fields')]
        self.notes_dict = {key: '' for _, key in notes_mapping}

        for note in notes:
            print(note)
            print("-----------------")
            for prefix, key in notes_mapping:
                if note['y:UMLNoteNode']['y:NodeLabel']['#text'].startswith(prefix):
                    self.notes_dict[key] = note['y:UMLNoteNode']['y:NodeLabel']['#text']
        self.start_node = start_node
        self.start_action = start_action
        self.states = states
        for state in states:
            print(state)
            print("--------------------")
            self.id_to_name[state.id] = state.name
            for trigger in state.trigs:
                if trigger.guard:
                    trigger.guard = trigger.guard.strip()

    def write_to_file(self, folder: str, extension : str):
        with open(os.path.join(folder, f'{self.sm_name}.{extension}'), 'w') as f:
            self.f = f
            self._insert_file_template('preamble_c.txt')
            self._write_constructor()
            self._write_initial()
            self._write_states_definitions_recursively(self.states[0], 'SMs::%s::SM' % self._sm_capitalized_name())
            self._insert_file_template('footer_c.txt')
            if self.notes_dict['raw_cpp_code']:
                self._insert_string('\n//Start of c code from diagram\n')
                self._insert_string('\n'.join(self.notes_dict['raw_cpp_code'].split('\n')[1:]) + '\n')
                self._insert_string('//End of c code from diagram\n\n\n')
            self.f = None

        with open(os.path.join(folder, '%s.h' % self.sm_name), 'w') as f:
            self.f = f
            self._insert_file_template('preamble_h.txt')

            if self.notes_dict['raw_h_code']:
                self._insert_string('//Start of h code from diagram\n')
                self._insert_string('\n'.join(self.notes_dict['raw_h_code'].split('\n')[1:]) + '\n')
                self._insert_string('//End of h code from diagram\n\n\n')

            self._write_full_line_comment('.$declare${SMs::STATE_MACHINE_CAPITALIZED_NAME}', 'v')
            self._write_full_line_comment('.${SMs::STATE_MACHINE_CAPITALIZED_NAME}', '.')
            self._insert_string('typedef struct {\n')
            self._insert_string('/* protected: */\n')
            self._insert_string('    QHsm super;\n')
            self._insert_string('\n')
            self._insert_string('/* public: */\n')
            constructor_fields: str = self.notes_dict['state_fields']
            self._insert_string('    ' + '\n    '.join(constructor_fields.split('\n')[1:]) + '\n')
            self._insert_string('} STATE_MACHINE_CAPITALIZED_NAME;\n\n')
            self._insert_string('/* protected: */\n')
            self._insert_string('QState STATE_MACHINE_CAPITALIZED_NAME_initial(STATE_MACHINE_CAPITALIZED_NAME * const me, void const * const par);\n')
            self._write_states_declarations_recursively(self.states[0])
            self._insert_string('\n#ifdef DESKTOP\n')
            self._insert_string(
                'QState STATE_MACHINE_CAPITALIZED_NAME_final(STATE_MACHINE_CAPITALIZED_NAME * const me, QEvt const * const e);\n')
            self._insert_string('#endif /* def DESKTOP */\n\n')
            self._write_full_line_comment('.$enddecl${SMs::STATE_MACHINE_CAPITALIZED_NAME}', '^')
            self._insert_string('\nstatic STATE_MACHINE_CAPITALIZED_NAME STATE_MACHINE_NAME; /* the only instance of the STATE_MACHINE_CAPITALIZED_NAME class */\n\n\n\n')

            self._insert_string('typedef struct STATE_MACHINE_NAMEQEvt {\n')
            self._insert_string('    QEvt super;\n')
            event_fields: str = self.notes_dict['event_fields']
            self._insert_string('    ' + '\n    '.join(event_fields.split('\n')[1:]) + '\n')
            self._insert_string('} STATE_MACHINE_NAMEQEvt;\n\n')
            self._insert_string(get_enum(self.player_signal) + '\n')
            self._insert_string('extern QHsm * const the_STATE_MACHINE_NAME; /* opaque pointer to the STATE_MACHINE_NAME HSM */\n\n')
            self._write_full_line_comment('.$declare${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', 'v')
            self._write_full_line_comment('.${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', '.')
            self._insert_string('void STATE_MACHINE_CAPITALIZED_NAME_ctor(')
            constructor_fields: str = self.notes_dict['constructor_fields']
            if constructor_fields:
                self._insert_string(
                    '\n    ' + ',\n    '.join(constructor_fields.replace(';', '').split('\n')[1:]) + ');\n')
            else:
                self._insert_string('void);\n')
            self._write_full_line_comment('.$enddecl${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', '^')
            self._insert_file_template('footer_h.txt')
            self.f = None

    def _write_constructor(self):
        self._write_full_line_comment('.$define${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', 'v')
        self._write_full_line_comment('.${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', '.')
        self._insert_string('void STATE_MACHINE_CAPITALIZED_NAME_ctor(')
        constructor_fields: str = self.notes_dict['constructor_fields']
        if constructor_fields:
            self._insert_string('\n    ' + ',\n    '.join(constructor_fields.replace(';', '').split('\n')[1:]) + ')\n')
            self._insert_string('{\n')
        else:
            self._insert_string('void) {\n')
        self._insert_string('    STATE_MACHINE_CAPITALIZED_NAME *me = &STATE_MACHINE_NAME;\n')
        constructor_code: str = self.notes_dict['constructor_code']
        self._insert_string('     ' + '\n    '.join(constructor_code.replace('\r', '').split('\n')[1:]))
        self._insert_string('\n')
        self._insert_string('    QHsm_ctor(&me->super, Q_STATE_CAST(&STATE_MACHINE_CAPITALIZED_NAME_initial));\n')
        self._insert_string('}\n')
        self._write_full_line_comment('.$enddef${SMs::STATE_MACHINE_CAPITALIZED_NAME_ctor}', '^')

    def _write_initial(self):
        self._write_full_line_comment('.$define${SMs::STATE_MACHINE_CAPITALIZED_NAME}', 'v')
        self._write_full_line_comment('.${SMs::STATE_MACHINE_CAPITALIZED_NAME}', '.')
        self._write_full_line_comment('.${SMs::STATE_MACHINE_CAPITALIZED_NAME::SM}', '.')
        self._insert_string(
            'QState STATE_MACHINE_CAPITALIZED_NAME_initial(STATE_MACHINE_CAPITALIZED_NAME * const me, void const * const par) {\n')
        self._insert_string('    /*.${SMs::STATE_MACHINE_CAPITALIZED_NAME::SM::initial} */\n')
        self._insert_string('    %s\n' % self.start_action)
        self._insert_string(
            '    return Q_TRAN(&STATE_MACHINE_CAPITALIZED_NAME_%s);\n' % self.id_to_name[self.start_node])
        self._insert_string('}\n')

    def _write_guard_comment(self, f, state_path: str, event_name: str, guard: str):
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

        return self._insert_string(prefix + shortened_guard + suffix)

    def _write_full_line_comment(self, text: str, filler: str):
        self._insert_string(('/*' + text.replace('STATE_MACHINE_NAME', self.sm_name).replace('STATE_MACHINE_CAPITALIZED_NAME', self._sm_capitalized_name()) + ' ').ljust(76, filler) + '*/\n')

    def _sm_capitalized_name(self) -> str:
        return self.sm_name[0].upper() + self.sm_name[1:]

    def _insert_string(self, s: str):
        self.f.write(re.sub('[ ]*\n', '\n',
                       s.replace('STATE_MACHINE_NAME', self.sm_name).replace('STATE_MACHINE_CAPITALIZED_NAME', self._sm_capitalized_name())))

    def _insert_file_template(self, filename: str):
        with open(os.path.join('src/compiler/fullgraphmlparser/templates', filename)) as input_file:
            for line in input_file.readlines():
                self._insert_string(line)

    def _write_states_definitions_recursively(self, state: State, state_path: str):
        state_path = state_path + '::' + state.name
        state_comment = '/*.${' + state_path + '} '
        state_comment = state_comment + '.' * (76 - len(state_comment)) + '*/\n'
        self.f.write(state_comment)
        self._insert_string('QState STATE_MACHINE_CAPITALIZED_NAME_%s(STATE_MACHINE_CAPITALIZED_NAME * const me, QEvt const * const e) {\n' % state.name)
        self._insert_string('    QState status_;\n')
        self._insert_string('    switch (e->sig) {\n')

        if state.name == 'global':
            self._insert_file_template('terminate_sig_c.txt')
        else:
            self._insert_string('        /*.${' + state_path + '} */\n')
            self._insert_string('        case Q_ENTRY_SIG: {\n')
            self._insert_string('\n'.join(['            ' + line for line in state.entry.split('\n')])  + '\n')
            self._insert_string('            status_ = Q_HANDLED();\n')
            self._insert_string('            break;\n')
            self._insert_string('        }\n')

            self._insert_string('        /*.${' + state_path + '} */\n')
            self._insert_string('        case Q_EXIT_SIG: {\n')
            self._insert_string('\n'.join(['            ' + line for line in state.exit.split('\n')])  + '\n')
            self._insert_string('            status_ = Q_HANDLED();\n')
            self._insert_string('            break;\n')
            self._insert_string('        }\n')

        name_to_triggers = defaultdict(list)
        name_to_position = {}

        for i, trigger in enumerate(state.trigs):
            if '?def' in trigger.name:
                continue

            name_to_triggers[trigger.name].append(trigger)
            name_to_position[trigger.name] = i

        triggers_merged: List[Tuple[str, List[Trigger]]] = sorted(
            [(name, name_to_triggers[name]) for name in name_to_triggers],
            key = lambda t: name_to_position[t[0]])

        for event_name, triggers in triggers_merged:
            self._insert_string('        /*.${%s::%s} */\n' % (state_path, event_name))
            self._insert_string('        case %s_SIG: {\n' % event_name)
            if len(triggers) == 1:
                if triggers[0].guard:
                    self._write_guard_comment(self.f, state_path, event_name, triggers[0].guard)
                    self._insert_string('            if (%s) {\n' % triggers[0].guard)
                    self._write_trigger(self.f, triggers[0], state_path, event_name, '    ')
                    self._insert_string('            }\n')
                    self._insert_string('            else {\n')
                    self._insert_string('                status_ = Q_UNHANDLED();\n')
                    self._insert_string('            }\n')
                else:
                    self._write_trigger(self.f, triggers[0], state_path, event_name)
            elif len(triggers) == 2:
                if triggers[0].guard == 'else':
                    triggers[0], triggers[1] = triggers[1], triggers[0]
                self._write_guard_comment(self.f, state_path, event_name, triggers[0].guard)
                self._insert_string('            if (%s) {\n' % triggers[0].guard)
                self._write_trigger(self.f, triggers[0], state_path, event_name, '    ')
                self._insert_string('            }\n')
                self._write_guard_comment(self.f, state_path, event_name, triggers[1].guard)
                self._insert_string('            else {\n')
                self._write_trigger(self.f, triggers[1], state_path, event_name, '    ')
                self._insert_string('            }\n')
            else:
                raise Exception('"else if" guards are not supported')
            self._insert_string('            break;\n')
            self._insert_string('        }\n')

        self._insert_string('        default: {\n')
        if state.parent:
            self._insert_string('            status_ = Q_SUPER(&STATE_MACHINE_CAPITALIZED_NAME_%s);\n' % state.parent.name)
        else:
            self._insert_string('            status_ = Q_SUPER(&QHsm_top);\n')
        self._insert_string('            break;\n')
        self._insert_string('        }\n')
        self._insert_string('    }\n')
        self._insert_string('    return status_;\n')
        self._insert_string('}\n')

        for child_state in state.childs:
            self._write_states_definitions_recursively(child_state, state_path)

        for trigger in state.trigs:
            if '?def' in trigger.name:
                continue
            if not trigger.name in self.all_signals:
                self.all_signals.append(trigger.name)

    def _write_states_declarations_recursively(self, state: State):
        self._insert_string('QState STATE_MACHINE_CAPITALIZED_NAME_%s(STATE_MACHINE_CAPITALIZED_NAME * const me, QEvt const * const e);\n' % state.name)
        for child_state in state.childs:
            self._write_states_declarations_recursively(child_state)

    def _write_trigger(self, f, trigger: Trigger, state_path: str, event_name: str, offset = ''):
        if trigger.action and not trigger.type == 'choice_start':
            self._insert_string('\n'.join(
                [offset + '            ' + line for line in trigger.action.split('\n')]) + '\n')
        if trigger.type == 'internal':
            self._insert_string(offset + '            status_ = Q_HANDLED();\n')
        elif trigger.type == 'external' or trigger.type == 'choice_result':
            self._insert_string(offset + '            status_ = Q_TRAN(&STATE_MACHINE_CAPITALIZED_NAME_%s);\n' % self.id_to_name[
                                    trigger.target])
        elif trigger.type == 'choice_start':
            target_choice_node = next((s for s in self.states if s.id == trigger.target and s.type == 'choice'), None)
            assert target_choice_node
            assert len(target_choice_node.trigs) == 2
            triggers = target_choice_node.trigs
            if triggers[0].guard == 'else':
                triggers[0], triggers[1] = triggers[1], triggers[0]
            triggers[0].action = trigger.action + triggers[0].action
            triggers[1].action = trigger.action + triggers[1].action
            self._write_guard_comment(self.f, state_path, event_name, triggers[0].guard)
            self._insert_string(offset + '            if (%s) {\n' % triggers[0].guard)
            self._write_trigger(self.f, triggers[0], state_path, event_name, offset + '    ')
            self._insert_string(offset + '            }\n')
            self._write_guard_comment(self.f, state_path, event_name, triggers[1].guard)
            self._insert_string(offset + '            else {\n')
            self._write_trigger(self.f, triggers[1], state_path, event_name, offset + '    ')
            self._insert_string(offset + '            }\n')
        else:
            raise Exception('Unknown trigger type: %s' % trigger.type)
