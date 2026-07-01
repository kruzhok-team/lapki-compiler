import os.path
import re
import string
from collections import defaultdict
from typing import List, Sequence, Tuple, Dict, DefaultDict

from aiofile import async_open
from compiler.fullgraphmlparser.stateclasses import (
    CodeGenerationException,
    ComputationFunction,
    ParserState,
    ParserTrigger,
    StateMachine,
    Labels,
    UnconditionalTransition,
    BaseParserVertex,
    Condition,
    GeneratorShallowHistory,
)
from compiler.fullgraphmlparser.graphml import *
from compiler.config import get_config
MODULE_PATH = os.path.join(get_config().module_directory, 'fullgraphmlparser')
IF_EXPRESSION = string.Template("""$offset if ($condition) {
$actions
$offset}""")

TRANSITION = string.Template(
    'status_ = Q_TRAN(&STATE_MACHINE_CAPITALIZED_NAME_$target);')

FINAL_ACTION = 'status_ = Q_HANDLED();'

ELSE_IF_EXPRESSION = string.Template("""$offset else if ($condition) {
$actions
$offset }""")

ELSE_EXPRESSION = string.Template("""$offset else {
$actions
$offset}
""")

DEFER = string.Template("""
$offset if (!signalDefer) {
$offset defer[defer_i] = $trigger_name;\n
$offset defer_i++;\n
$offset}
""")


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
    f = None
    all_signals = []
    userFlag = False  # Флаг на наличие кода для класса User

    def __init__(self,
                 state_machine: StateMachine,
                 create_setup=False,
                 create_loop=False) -> None:
        self.header_file_extension = state_machine.header_file_extension
        self.language = state_machine.language
        self.filename = 'sketch'
        self.create_loop = create_loop
        self.create_setup = create_setup
        self.sm_id = 'sketch'
        self.sm_name = state_machine.name
        self.player_signal = state_machine.signals
        self.computation_functions = state_machine.computation_functions

        self.notes: DefaultDict[str, List[str]] = defaultdict(list)

        for note in state_machine.notes:
            # Делаем так, чтобы при повторении меток, их содержимое
            # объединялось, а не перезаписывалось
            self.notes[note.label.value].append(note.content)

        self.start_node = state_machine.start_node
        self.start_action = state_machine.start_action
        self.global_state = state_machine.global_state
        self.shallow_history = self.__convert_local_history_to_dict(
            state_machine.shallow_history)
        self.initial_states = state_machine.initial_states
        self.choices = state_machine.choices
        self.final_states = state_machine.final_states

    def __convert_local_history_to_dict(
        self,
        shallow_history: List[GeneratorShallowHistory]
    ) -> Dict[str, GeneratorShallowHistory]:
        """
        Конвертировать массив локальных историй в словарь, где ключом является\
            id родителя.

        Словарь в дальнейшем используется при генерации состоянии, чтобы
        отслеживать, находится ли локальная история на уровне состояния.

        Вызываемые исключения:
        - `CodeGeneratorException` - если на одном уровне находится\
            более одной локальной истории
        """
        sh_dict: Dict[str, GeneratorShallowHistory] = {}

        for sh in shallow_history:
            is_exist = sh_dict.get(sh.parent) is not None
            if is_exist:
                raise CodeGenerationException(
                    f'У элемента {sh.parent} более одной'
                    ' дочерней локальной истории.'
                )
            sh_dict[sh.parent] = sh

        return sh_dict

    async def _write_unconditional_transition(
        self,
        transition: UnconditionalTransition,
        offset: str = ''
    ) -> None:
        actions = transition.action.split('\n')
        await self._insert_string(f'{offset}\n\t'.join(actions))
        await self._insert_string(offset + '\tstatus_ = Q_TRAN(&STATE_MACHINE_CAPITALIZED_NAME_%s);\n' % transition.target)

    async def _write_vertexes_declaration(self,
                                          vertexes: Sequence[BaseParserVertex]):
        for vertex in vertexes:
            await self._insert_string('QState STATE_MACHINE_CAPITALIZED_NAME_%s(STATE_MACHINE_CAPITALIZED_NAME * const me, QEvt const *const e);\n' % vertex.id)

    async def _generate_condition(self,
                                  id: str,
                                  type: str,
                                  triggers: Sequence[Condition],
                                  parent: str | None,
                                  offset='\t\t') -> str:
        """Generate if else code using IF_EXPRESSIONS,\
            ELSE_IF_EXPRESSION, ELSE_EXPRESSION templates."""
        if parent is not None:
            propagate_expression = f'status_ = Q_SUPER(&STATE_MACHINE_CAPITALIZED_NAME_{parent});'
        else:
            propagate_expression = 'status_ = Q_UNHANDLED();'
        if len(triggers) == 0:
            return offset + propagate_expression

        else_count = sum([trigger.guard == 'else' for trigger in triggers])
        if (else_count > 1):
            raise CodeGenerationException(
                f'{type}({id}): Может быть лишь один переход с условием else.')
        start_trigger_index = 0
        start_trigger = triggers[0]
        for i in range(len(triggers)):
            if triggers[i].guard != 'else':
                start_trigger_index = i
                start_trigger = triggers[i]
                break
        actions = IF_EXPRESSION.safe_substitute({
            'condition': start_trigger.guard,
            'actions': start_trigger.action,
            'offset': offset
        }) + '\n'

        else_condition: Condition | None = None

        for i in range(0, len(triggers)):
            if start_trigger_index == i:
                continue
            transition = triggers[i]
            if transition.guard == 'else':
                else_condition = transition
                continue
            actions += ELSE_IF_EXPRESSION.safe_substitute({
                'condition': transition.guard,
                'actions': transition.action,
                'offset': offset
            }) + '\n'
        if else_condition is not None:
            actions += ELSE_EXPRESSION.safe_substitute({
                'actions': else_condition.action,
                'offset': offset
            }) + '\n'
        else:
            actions += ELSE_EXPRESSION.safe_substitute({
                'actions': propagate_expression,
                'offset': offset
            }) + '\n'
        return actions

    async def _write_choice_vertex_definition(self):
        """Generate choice vertex definition."""
        for choice in self.choices:
            for transition in choice.transitions:
                transition.action += '\t' + \
                    TRANSITION.safe_substitute(
                        {'target': transition.target})
            await self._write_vertex_definition(
                await self._generate_condition(
                    choice.id,
                    'Псевдосостояние выбора',
                    choice.transitions,
                    choice.parent
                ),
                choice,
                'Choice'
            )

    async def _write_initial_vertexes_definition(self) -> None:
        """Write initial vertexes definition."""
        for initial in self.initial_states:
            actions = initial.transition.action
            actions += '\n'
            await self._write_vertex_definition(f'status_ = Q_TRAN(&STATE_MACHINE_CAPITALIZED_NAME_{initial.transition.target});\n',
                                                initial,
                                                'Initial')

    async def _write_vertex_definition(self, vertex_actions: str,
                                       vertex: BaseParserVertex,
                                       vertex_type: str) -> None:
        """
        Write function-vertex definition.

        vertex_actions: actions, that will be added to Q_VERTEX_SIG.
        This function add `inVertex = false;` at the end
        """
        await self._write_full_line_comment(
            f'{vertex_type} pseudostate {vertex.id}', ' ')
        await self._insert_string('QState STATE_MACHINE_CAPITALIZED_NAME_%s(STATE_MACHINE_CAPITALIZED_NAME * const me, QEvt const * const e) {\n' % vertex.id)
        await self._insert_string('         QState status_;\n')
        await self._insert_string('         switch(e -> sig) {\n')
        await self._insert_string('            case Q_ENTRY_SIG: {\n')
        await self._insert_string('\n              status_ = Q_HANDLED();')
        await self._insert_string('\n              inVertex = true;')
        await self._insert_string('\n              break;')
        await self._insert_string('\n          }')
        await self._insert_string('\n            case Q_EXIT_SIG: {')
        await self._insert_string('\n              inVertex = false;')
        await self._insert_string('\n              status_ = Q_HANDLED();')
        await self._insert_string('\n              break;')
        await self._insert_string('\n          }')
        await self._insert_string('\n          case Q_VERTEX_SIG: {\n')
        actions = vertex_actions.split('\n')
        await self._insert_string('\n              '.join(actions))
        await self._insert_string('\n              inVertex = false;')
        await self._insert_string('\n              break;')
        await self._insert_string('\n          }')
        await self._insert_string('\n          default: {')
        await self._insert_string(f'\n              status_ = Q_SUPER(&STATE_MACHINE_CAPITALIZED_NAME_{vertex.parent});')
        await self._insert_string('\n              break;')
        await self._insert_string('\n          }')
        await self._insert_string('\n         }\n')
        await self._insert_string('\n         return status_;')
        await self._insert_string('\n}\n\n')

    async def _write_local_history_initialization(self) -> None:
        """
        Генерация иниализации массива локальной истории для sketch.h.

        ```cpp
        // Пример кода инициализации
        QStateHandler shallowHistory[3] = {
            Q_STATE_CAST(Sketch_pixtlgycbblxtahjlzhl),
            Q_STATE_CAST(Sketch_pixtlgycbblxtahjlzhl),
            Q_STATE_CAST(Sketch_global) // если default_value отсутствует
        };
        ```
        """
        def get_casted_state(target_id: str | None) -> str:
            if target_id is None:
                target_id = 'QHsm_top'
            return f'\tQ_STATE_CAST(STATE_MACHINE_CAPITALIZED_NAME_{target_id})'
        insert_strings = [
            f'QStateHandler shallowHistory[{len(self.shallow_history)}] = ' + '{'
        ]
        shallow_history_sorted_by_index = sorted(
            self.shallow_history.values(), key=lambda lh: lh.index)
        insert_shallows = [
            f'{get_casted_state(lh.default_value)},'
            for lh in shallow_history_sorted_by_index]
        insert_strings.extend(insert_shallows)
        insert_strings.append('};')

        await self._insert_string('\n'.join(insert_strings) + '\n')

    async def _write_local_history_definition(self):
        """
        Генерация тела локальных историй и запись их в файл.

        ```cpp
        QState local_history_id(Sketch * const me, QEvt const * const e) {
            switch (e->sig) {
                // ...стандартный код

                case Q_VERTEX_SIG: {
                    status_ = Q_TRAN(shallowHistory[{shallow_history.index}])
                    inVertex = false;   // IMPL
                    break;
                }

                // ...стандартный код
            }
        }
        ```
        """
        for shallow_history in self.shallow_history.values():
            await self._write_vertex_definition(
                f'status_ = Q_TRAN(shallowHistory[{shallow_history.index}]);\n',
                shallow_history,
                'Shallow history')

    async def _write_final_states_definition(self):
        for final in self.final_states:
            await self._write_full_line_comment(f'Final pseudostate {final.id}', ' ')
            await self._insert_string('QState STATE_MACHINE_CAPITALIZED_NAME_%s(STATE_MACHINE_CAPITALIZED_NAME * const me, QEvt const * const e) {\n' % final.id)
            await self._insert_string('         QState status_;\n')
            await self._insert_string(f'         {FINAL_ACTION}\n')
            await self._insert_string('         return status_;\n')
            await self._insert_string('}\n\n')

    async def write_to_file(self, folder: str, extension: str):
        """
        Главная функция для генерации проекта.

        Генерирует файлы:
        - filename.extension
        - filename.header_file_extension\
            (header_file_extension передается в конструкторе)
        """
        async with async_open(os.path.join(folder, f'{self.filename}.{extension}'), 'w') as f:
            self.f = f
            await self._insert_file_template(f'preamble_c.txt')
            await self._write_constructor()
            await self._write_initial()
            await self._write_states_definitions_recursively(
                self.global_state,
                'SMs::%s::SM' % self._sm_capitalized_name()
            )
            await self._write_initial_vertexes_definition()
            await self._write_choice_vertex_definition()
            await self._write_final_states_definition()
            await self._write_local_history_definition()
            await self._write_computation_functions()
            setup_notes = self.notes[Labels.SETUP.value]
            if setup_notes or self.create_setup:
                await self._insert_string('\nvoid setup() {')
                await self._insert_string('\n\t' + '\n\t'.join(
                    setup_notes)
                )
                await self._insert_string('\n\tSTATE_MACHINE_CAPITALIZED_NAME_ctor();')
                await self._insert_string('\n\tQEvt event;')
                await self._insert_string(f'\n\tQMsm_init(the_{self.sm_id}, &event);')
                await self._insert_string('\n}')
            loop_notes = self.notes[Labels.LOOP.value]
            if loop_notes or self.create_loop:
                await self._insert_string('\nvoid loop() {')
                await self._insert_file_template('q_vertex_sig.txt')
                await self._insert_file_template('defer_loop.txt')
                await self._insert_string('\n\t' + '\n\t'.join(loop_notes))
                await self._insert_string('\n}')
            main_notes = self.notes[Labels.MAIN_FUNCTION.value]
            if main_notes:
                await self._insert_string(main_notes)
            raw_code = self.notes[Labels.CPP.value]
            if raw_code:
                await self._insert_string('\n'.join(raw_code) + '\n')
            self.f = None

        async with async_open(os.path.join(folder, f'{self.filename}.{self.header_file_extension}'), 'w') as f:
            self.f = f

            await self._insert_file_template(f'preamble_{self.header_file_extension}.txt')
            # if self.userFlag:
            # await self._insert_string('#include "User.h"\n')
            h_notes = self.notes[Labels.H_INCLUDE.value]
            if h_notes:
                await self._insert_string('//Start of h code from diagram\n')
                await self._insert_string('\n'.join(h_notes) + '\n')
                await self._insert_string('//End of h code from diagram\n\n\n')

            await self._insert_string('typedef struct {\n')
            await self._insert_string('/* protected: */\n')
            await self._insert_string('    QHsm super;\n')
            await self._insert_string('\n')
            await self._insert_string('/* public: */\n')
            state_fields = self.notes[Labels.STATE_FIELDS.value]
            await self._insert_string('    ' + '\n    '.join(state_fields) + '\n')
            await self._insert_string('} STATE_MACHINE_CAPITALIZED_NAME;\n\n')
            await self._insert_string('/* protected: */\n')
            await self._insert_string('QState DEFAULT_STATE_MACHINE_CAPITALIZED_NAME_initial(STATE_MACHINE_CAPITALIZED_NAME * const me, void const * const par);\n')
            await self._write_states_declarations_recursively(self.global_state)
            await self._write_vertexes_declaration([
                *self.initial_states,
                *self.choices,
                *self.final_states,
                *list(
                    self.shallow_history.values()
                )
            ])
            await self._insert_string('\n#ifdef DESKTOP\n')
            await self._insert_string('#endif /* def DESKTOP */\n\n')
            await self._insert_string('extern QHsm * const the_STATE_MACHINE_LOWERED_NAME; /* opaque pointer to the STATE_MACHINE_LOWERED_NAME HSM */\n\n')

            await self._insert_string('typedef struct STATE_MACHINE_LOWERED_NAMEQEvt {\n')
            await self._insert_string('    QEvt super;\n')
            event_fields = self.notes[Labels.EVENT_FIELDS.value]
            await self._insert_string('    ' + '\n    '.join(event_fields) + '\n')
            await self._insert_string('} STATE_MACHINE_LOWERED_NAMEQEvt;\n\n')
            await self._insert_string(
                get_enum(list(self.player_signal)) + '\n'
            )
            await self._insert_string('\nstatic STATE_MACHINE_CAPITALIZED_NAME STATE_MACHINE_LOWERED_NAME; /* the only instance of the STATE_MACHINE_CAPITALIZED_NAME class */\n\n\n\n')
            await self._insert_string('void STATE_MACHINE_CAPITALIZED_NAME_ctor(')
            constructor_fields = self.notes[Labels.CTOR_FIELDS.value]
            if constructor_fields:
                await self._insert_string(
                    '\n    ' + ',\n    '.join(constructor_fields)
                    .replace(';', '') + ');\n'
                )
            else:
                await self._insert_string('void);\n')
            await self._write_local_history_initialization()
            declare_h_notes = self.notes[Labels.H.value]
            if declare_h_notes:
                await self._insert_string('//Start of h code from diagram\n')
                await self._insert_string('\n'.join(declare_h_notes) + '\n')
                await self._insert_string('//End of h code from diagram\n\n\n')
            await self._insert_file_template(f'footer_{self.header_file_extension}.txt')
            self.f = None

    async def _write_constructor(self):
        await self._insert_string('void STATE_MACHINE_CAPITALIZED_NAME_ctor(')
        constructor_fields = self.notes[Labels.CTOR_FIELDS.value]
        if constructor_fields:
            await self._insert_string(
                '\n    ' +
                ',\n    '.join(constructor_fields).replace(';', '') + ')\n'
            )
            await self._insert_string('{\n')
        else:
            await self._insert_string('void) {\n')
        await self._insert_string('    STATE_MACHINE_CAPITALIZED_NAME *me = &STATE_MACHINE_LOWERED_NAME;\n')
        constructor_code = self.notes[Labels.CTOR.value]
        await self._insert_string(
            '     ' +
            '\n    '.join(constructor_code).replace('\r', '')
        )
        await self._insert_string('\n')
        await self._insert_string('    QHsm_ctor(&me->super, Q_STATE_CAST(&DEFAULT_STATE_MACHINE_CAPITALIZED_NAME_initial));\n')
        await self._insert_string('}\n')

    async def _write_initial(self):
        await self._insert_string(
            'QState DEFAULT_STATE_MACHINE_CAPITALIZED_NAME_initial(STATE_MACHINE_CAPITALIZED_NAME * const me, void const * const par) {\n')
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
        await self._insert_string(('/*' + text.replace('STATE_MACHINE_NAME', self.sm_id).replace('STATE_MACHINE_CAPITALIZED_NAME', self._sm_capitalized_name()) + ' ').ljust(76, filler) + '*/\n')

    def _sm_capitalized_name(self) -> str:
        return self.sm_id[0].upper() + self.sm_id[1:]

    def _sm_lowered_name(self) -> str:
        return self.sm_id[0].lower() + self.sm_id[1:]

    async def _insert_string(self, s: str | List[str]):
        if isinstance(s, list):
            s = '\n\t'.join(s)
        await self.f.write(re.sub('[ ]*\n', '\n',
                                  s.replace('STATE_MACHINE_LOWERED_NAME', self._sm_lowered_name()).replace('STATE_MACHINE_CAPITALIZED_NAME', self._sm_capitalized_name()).replace('STATE_MACHINE_NAME', self.sm_id).replace('HEADER_EXTENSION', self.header_file_extension)))

    async def _insert_file_template(self, filename: str):
        async with async_open(os.path.join(MODULE_PATH, 'templates', filename)) as input_file:
            async for line in input_file:
                await self._insert_string(str(line))

    def _generate_defer(self, trigger_name: str, offset='\t\t') -> str:
        return DEFER.safe_substitute({'trigger_name': trigger_name + '_SIG', 'offset': offset})

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
            await self._insert_string('            stateChanged = false;\n')
            # Ставим флаг, означающий необходимость вызова Q_VERTEX_SIG
            if state.initial_state:
                await self._insert_string('            inVertex = true;\n')
            else:
                await self._insert_string('            inVertex = false;\n')

            # Если локальная история находится на одном уровне,
            # то добавляем посещение состояния в массив
            if state.parent is not None:
                shallow_history = self.shallow_history.get(state.parent)
                if shallow_history is not None:
                    await self._insert_string(
                        f'shallowHistory[{shallow_history.index}] '
                        '= Q_STATE_CAST(STATE_MACHINE_CAPITALIZED_NAME_'
                        f'{state.id});')

            await self._insert_string('            status_ = Q_HANDLED();\n')
            await self._insert_string('\n'.join(['            ' + line for line in state.entry.split('\n')]) + '\n')
            await self._insert_string('            break;\n')
            await self._insert_string('        }\n')
            if state.initial_state:
                await self._insert_string('        case Q_VERTEX_SIG:')
                await self._insert_string(' {\n')
                await self._insert_string(f'            status_ = Q_TRAN(&STATE_MACHINE_CAPITALIZED_NAME_{state.initial_state});\n')
                await self._insert_string('            inVertex = false;\n')
                await self._insert_string('            break;\n')
                await self._insert_string('        }\n')
            else:
                await self._insert_string('\n')
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
            await self._insert_string('        /*.${%s::%s} */\n' % (
                state_path,
                event_name)
            )
            await self._insert_string('        case %s_SIG: {\n' % event_name)

            for trigger in triggers:
                trigger.action = self._generate_trigger(trigger, state)
            await self._insert_string(
                await self._generate_condition(
                    state.id if state.name is None else state.name,
                    'Состояние',
                    triggers,
                    state.parent,
                    '\t\t\t'
                )
            )
            await self._insert_string('            break;\n')
            await self._insert_string('        }\n')
        await self._insert_string('        default: {\n')
        if state.parent:
            await self._insert_string('            status_ = Q_SUPER(&STATE_MACHINE_CAPITALIZED_NAME_%s);\n' % state.parent)
        else:
            await self._insert_string('            status_ = Q_SUPER(&QHsm_top);\n')
        await self._insert_string('            break;\n')
        await self._insert_string('        }\n')
        await self._insert_string('    }\n')
        await self._insert_string('    return status_;\n')
        await self._insert_string('}\n')
        for child_state in state.children:
            await self._write_states_definitions_recursively(child_state, state_path)

        for trigger in state.trigs:
            if '?def' in trigger.name:
                continue
            if trigger.name not in self.all_signals:
                self.all_signals.append(trigger.name)

    async def _write_states_declarations_recursively(self, state: ParserState):
        await self._insert_string('QState STATE_MACHINE_CAPITALIZED_NAME_%s(STATE_MACHINE_CAPITALIZED_NAME * const me, QEvt const * const e);\n' % state.id)
        for child_state in state.children:
            await self._write_states_declarations_recursively(child_state)

    def _generate_trigger(self,
                          trigger: ParserTrigger,
                          state: ParserState,
                          offset='') -> str:
        """Generate trigger body."""
        actions = ''
        if trigger.defer:
            actions += self._generate_defer(trigger.name)
        elif trigger.action and not trigger.type == 'choice_start':
            actions += '\n'.join(
                [offset + '            ' + line for line in trigger.action.split('\n')]) + '\n'
        if trigger.type == 'internal':
            if trigger.propagate:
                if state.parent:
                    actions += '            status_ = Q_SUPER(&STATE_MACHINE_CAPITALIZED_NAME_%s);\n' % state.parent
                else:
                    actions += '            status_ = Q_SUPER(&QHsm_top);\n'
            else:
                actions += offset + '            status_ = Q_HANDLED();\n'
        elif trigger.type == 'external' or trigger.type == 'choice_result':
            actions += offset + '            stateChanged = true;\n'
            actions += offset + \
                '            status_ = Q_TRAN(&STATE_MACHINE_CAPITALIZED_NAME_%s);\n' % trigger.target
        else:
            raise Exception('Unknown trigger type: %s' % trigger.type)

        return actions

    def _generate_function_code(self, func: ComputationFunction) -> tuple[str, str]:
        """
        Генерирует объявление и определение C++ функции для одного ComputationFunction.
        Автоматически определяет внешние параметры на основе входов блоков.
        """
        # Топологическая сортировка
        dependencies = {block.id: [] for block in func.blocks}
        in_degree = {block.id: 0 for block in func.blocks}
        for conn in func.connections:
            if conn.target_block_id in dependencies:
                dependencies[conn.target_block_id].append(conn.source_block_id)
                in_degree[conn.target_block_id] += 1

        from collections import deque
        queue = deque([bid for bid, deg in in_degree.items() if deg == 0])
        sorted_blocks = []
        while queue:
            bid = queue.popleft()
            sorted_blocks.append(bid)
            for dep in dependencies.get(bid, []):
                in_degree[dep] -= 1
                if in_degree[dep] == 0:
                    queue.append(dep)

        if len(sorted_blocks) != len(func.blocks):
            raise CodeGenerationException(f'Циклическая зависимость в функции {func.name}')

        # Определение внешних параметров
        block_ids = {b.id for b in func.blocks}
        external_params = set()
        for block in func.blocks:
            for port, value in block.inputs.items():
                if isinstance(value, (int, float)):
                    continue
                if isinstance(value, str):
                    if value.isdigit() or (value.startswith('-') and value[1:].isdigit()):
                        continue
                    if value in block_ids:
                        continue
                    external_params.add(value)

        # Преобразование параметров в типы
        param_list = []
        for p in external_params:
            if p == 'data' or 'array' in p or 'buf' in p:
                param_list.append(('const int8_t*', p))
            elif p == 'size' or 'len' in p:
                param_list.append(('size_t', p))
            elif p == 'coeff':
                param_list.append(('float', p))
            else:
                param_list.append(('int32_t', p))

        params_str = ', '.join([f'{typ} {name}' for typ, name in param_list])
        available = {name: name for _, name in param_list}

        # Генерация тела
        temp_vars = {}
        body_lines = []

        # Определяем, какой блок является выходным (его результат возвращается)
        used_as_source = {conn.source_block_id for conn in func.connections}
        output_block_id = None
        for block in func.blocks:
            if block.id not in used_as_source:
                output_block_id = block.id
                break
        if output_block_id is None and func.blocks:
            output_block_id = sorted_blocks[-1] if sorted_blocks else None

        for bid in sorted_blocks:
            block = next(b for b in func.blocks if b.id == bid)

            # Обработка блока abs (модифицирует массив на месте)
            if block.type == 'abs':
                data_param = None
                size_param = None
                for port, value in block.inputs.items():
                    if port == 'data' and value in available:
                        data_param = available[value]
                    elif port == 'size' and value in available:
                        size_param = available[value]
                if data_param is None:
                    data_param = 'data'
                if size_param is None:
                    size_param = 'size'
                body_lines.append(f'    func_abs({data_param}, {size_param});')
                continue

            # Формируем аргументы для вызова
            args = []
            for port, value in block.inputs.items():
                if value in available:
                    args.append(available[value])
                elif value in temp_vars:
                    args.append(temp_vars[value])
                else:
                    args.append(value)

            # Генерация вызова в зависимости от типа
            if block.type == 'sum':
                if len(args) >= 2:
                    array_arg, size_arg = args[0], args[1]
                else:
                    array_arg, size_arg = 'data', 'size'
                body_lines.append(f'    int32_t temp_{bid} = func_sum({array_arg}, {size_arg});')
            elif block.type == 'smooth':
                coeff = block.parameters.get('coeff', '0.4f')
                if len(args) >= 1:
                    value_arg = args[0]
                else:
                    value_arg = '0'
                # Для smooth нужен указатель на prev – создаём статическую переменную
                static_var = f'static int32_t prev_{bid} = 0;'
                body_lines.append(f'    {static_var}')
                body_lines.append(f'    int32_t temp_{bid} = func_smooth({value_arg}, {coeff}, &prev_{bid});')
            elif block.type == 'greater':
                if len(args) >= 2:
                    a, b = args[0], args[1]
                else:
                    a, b = '0', '0'
                body_lines.append(f'    int32_t temp_{bid} = func_greater({a}, {b}) ? 1 : 0;')
            elif block.type == 'const':
                const_value = block.parameters.get('value', '0')
                body_lines.append(f'    int32_t temp_{bid} = {const_value};')
            else:
                body_lines.append(f'    // Неизвестный блок {block.type} (id={bid})')

            temp_vars[bid] = f'temp_{bid}'
            # Добавляем имя блока в доступные переменные для последующих блоков
            available[bid] = f'temp_{bid}'

        # Формирование сигнатуры и тела
        if output_block_id and output_block_id in temp_vars:
            return_expr = temp_vars[output_block_id]
        else:
            return_expr = '0'

        signature = f'int32_t {func.name}({params_str})'
        declaration = f'{signature};'
        definition = f'{signature} {{\n'
        definition += '\n'.join(body_lines)
        definition += f'\n    return {return_expr};\n'
        definition += '}'

        return declaration, definition


    async def _write_computation_functions(self):
        if not self.computation_functions:
            return

        # Генерация отдельных функций
        declarations = []
        definitions = []
        for func in self.computation_functions:
            decl, defin = self._generate_function_code(func)
            declarations.append(decl)
            definitions.append(defin)

        # Генерация класса Func
        class_decl, class_def = self._generate_func_class()
        if class_decl:
            declarations.append(class_decl)
        if class_def:
            definitions.append(class_def)

        if declarations:
            self.notes[Labels.USER_FUNC_H.value].append(
                '\n// Вычислительные функции\n' + '\n'.join(declarations)
            )
        if definitions:
            self.notes[Labels.USER_FUNC_C.value].append(
                '\n// Определения вычислительных функций\n' + '\n'.join(definitions)
            )

    def _generate_func_class(self) -> tuple[str, str]:
        """
        Генерирует объявление и определение класса Func.
        Метод call вызывает все сгенерированные функции, передавая им data и size,
        если они присутствуют в сигнатуре функции.
        """
        if not self.computation_functions:
            return '', ''

        class_decl = """
        class Func {
        public:
            static bool withNoise;
            int32_t prev;

            void call(int8_t* data, size_t size);
        };
        """
        static_def = "bool Func::withNoise = false;\n"

        call_body = []
        for func in self.computation_functions:
            # Проверяем, использует ли функция параметры data и size
            uses_data = any(
                value == 'data' for block in func.blocks for value in block.inputs.values()
            )
            uses_size = any(
                value == 'size' for block in func.blocks for value in block.inputs.values()
            )

            if uses_data and uses_size:
                call_body.append(f'    (void){func.name}(data, size);')
            elif uses_data:
                call_body.append(f'    (void){func.name}(data, 60);')
            else:
                call_body.append(f'    (void){func.name}();')

        method_def = f"""
        void Func::call(int8_t* data, size_t size) {{
            // Сгенерированные вызовы всех вычислительных функций
            {chr(10).join(call_body)}
            }}
        """
        return class_decl, static_def + method_def