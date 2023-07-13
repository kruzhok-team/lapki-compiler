import os
import re
import sys
from dataclasses import dataclass, field
from typing import Dict, List, Optional

import clang.cindex
from lxml import etree

import create_graphml
import graphmltoqm
import qm

try:
    clang_index = clang.cindex.Index.create()
except:
    # Hack to support linux (e.g. Github Actions)
    clang.cindex.Config.set_library_file('/usr/lib/llvm-8/lib/libclang.so.1')
    clang_index = clang.cindex.Index.create()

# Зависимости:
# - clang/llvm
#   Можно установить через 'choco install llvm'.
# - clang python bindings
#   Можно установить через 'pip install clang'.
#
# Запуск:
#   py -3 cpp_to_graphml.py <путь к cpp-файлу диаграммы>


@dataclass
class EventHandler:
    state_from: str
    state_to: str
    event_type: str
    condition: Optional[str]
    statements: List[str]


@dataclass
class State:
    state_name: str
    parent_state_name: Optional[str] = None
    event_handlers: List[EventHandler] = field(default_factory=list)
    child_states: List['State'] = field(default_factory=list)


@dataclass
class StateMachineCpp:
    constructor_code: str = ''
    initial_code: str = ''
    initial_state: str = ''
    states: Dict[str, State] = field(default_factory=dict)


@dataclass
class StateMachineHeader:
    raw_h_code: str = ''
    state_fields: str = ''
    event_fields: str = ''
    constructor_fields: str = ''


@dataclass
class StateMachine(StateMachineCpp, StateMachineHeader):
    pass


class ParsingContext:
    state_machine_name: str
    file_path: str
    _file_content: str

    def __init__(self, file_path: str):
        filename = os.path.splitext(os.path.basename(file_path))[0]
        self.state_machine_name = filename[:1].upper() + filename[1:]
        self.file_path = file_path
        with open(file_path, 'r', newline='') as f:
            self._file_content = ''.join(f.readlines())

    def GetNodeText(self, node: clang.cindex.Cursor) -> str:
        source_range = node.extent
        return self._file_content[source_range.start.offset:source_range.end.offset + 1]

    def RemoveStateMachinNamePrefix(self, s: str) -> str:
        return s[(len(self.state_machine_name) + 1):]

    def CamelCaseStateMachinName(self) -> str:
        return self.state_machine_name[0].lower() + self.state_machine_name[1:]


class StateMachineParser:
    def __init__(self, cpp_file_path: str):
        self.cpp_parser = CppParser(cpp_file_path = cpp_file_path)
        self.header_parser = HeaderParser(header_file_path = os.path.splitext(cpp_file_path)[0]+'.h')

    def Parse(self):
        cpp_parse_result = self.cpp_parser.Parse()
        header_parse_result = self.header_parser.Parse()
        return StateMachine(constructor_code=cpp_parse_result.constructor_code, states=cpp_parse_result.states,
                            initial_code=cpp_parse_result.initial_code, initial_state=cpp_parse_result.initial_state,
                            raw_h_code=header_parse_result.raw_h_code, state_fields=header_parse_result.state_fields,
                            event_fields=header_parse_result.event_fields,
                            constructor_fields=header_parse_result.constructor_fields)


class CppParser:
    def __init__(self, cpp_file_path: str):
        translationUnit = clang_index.parse(cpp_file_path)
        self.ctx = ParsingContext(cpp_file_path)
        self.root_node = translationUnit.cursor
        self.result = StateMachineCpp()

    def Parse(self) -> StateMachineCpp:
        self._TraverseAST(self.root_node)
        self._UpdateChilds()
        return self.result

    def _UpdateChilds(self):
        for state_name in self.result.states:
            state = self.result.states[state_name]
            if state.parent_state_name:
                self.result.states[state.parent_state_name].child_states.append(state)

    def _TraverseAST(self, node):
        if self._IsStateFunction(node):
            state = StateParser(self.ctx, node).Parse()
            self.result.states[state.state_name] = state
#            print('''
# State discovered:
# name = %s, parent = %s, handlers = %s
#            ''' % (state.state_name, state.parent_state_name, state.handlers.keys()))

        if self._IsCtorFunction(node):
            for childNode in node.get_children():
                if childNode.kind == clang.cindex.CursorKind.COMPOUND_STMT:
                    constructor_statements = []
                    for childChildNode in childNode.get_children():
                        node_text = self.ctx.GetNodeText(childChildNode)
                        if '\n' in node_text:
                            offset = childChildNode.extent.start.column - 1
                            unindent = lambda s: s[offset:]
                            lines = node_text.split('\n')
                            lines = [lines[0]] + [unindent(l) for l in lines[1:]]
                            node_text = '\n'.join(lines)
                        constructor_statements.append(node_text)

                    # We skip the very first statement as it always 'OregonPlayer *me = &oregonPlayer;'
                    #  which added automatically
                    # We also skip the last as it's parent constructor call which is also added automatically
                    self.result.constructor_code = '\n'.join(constructor_statements[1:-1]).rstrip()

                    break

        # Only 1/2-line functions are supported at the moment
        if self._IsInitialFunction(node):
            body_node = list(node.get_children())[-1]
            child_nodes = list(body_node.get_children())
            assert len(child_nodes) in [1, 2]
            self.result.initial_code = self.ctx.GetNodeText(child_nodes[0])
            r = r'Q_TRAN\(&%s_([a-z_]*)\);' % self.ctx.state_machine_name
            self.result.initial_state = re.search(r, self.ctx.GetNodeText(child_nodes[-1]))[1]

        for childNode in node.get_children():
            self._TraverseAST(childNode)

    def _IsCtorFunction(self, node):
        return (node.kind == clang.cindex.CursorKind.FUNCTION_DECL and
                node.spelling == self.ctx.state_machine_name + '_ctor')

    def _IsInitialFunction(self, node):
        return (node.kind == clang.cindex.CursorKind.FUNCTION_DECL and
                node.spelling == self.ctx.state_machine_name + '_initial' and
                list(node.get_children())[-1].kind == clang.cindex.CursorKind.COMPOUND_STMT)

    def _IsStateFunction(self, node):
        return (node.kind == clang.cindex.CursorKind.FUNCTION_DECL and
                node.spelling.startswith(self.ctx.state_machine_name + '_') and
                not node.spelling.endswith('_ctor'))


class HeaderParser:
    def __init__(self, header_file_path: str):
        self.header_file_path = header_file_path
        self.ctx = ParsingContext(header_file_path)
        self.root_node = clang_index.parse(header_file_path).cursor
        self.result = StateMachineHeader()

    def Parse(self) -> str:
        self._ExtractHCode()
        for node in self.root_node.get_children():
            if node.kind == clang.cindex.CursorKind.STRUCT_DECL:
                attributes = list(node.get_children())
                if node.displayname == self.ctx.CamelCaseStateMachinName() + 'QEvt':
                    assert self.ctx.GetNodeText(attributes[0]) == 'QEvt super;'
                    self.result.event_fields = '\n'.join([self.ctx.GetNodeText(attr) for attr in attributes[1:]])

                if attributes and attributes[0].type.spelling == 'QHsm':
                    self.result.state_fields = '\n'.join([self.ctx.GetNodeText(attr) for attr in attributes[1:]])

            if node.kind == clang.cindex.CursorKind.FUNCTION_DECL \
                    and node.spelling == self.ctx.state_machine_name + '_ctor':
                # In function declaration last symbol is either ',' or '
                convert = lambda attr: '%s %s;' % (attr.type.spelling, attr.spelling)
                self.result.constructor_fields = '\n'.join([convert(attr) for attr in node.get_children()])

        return self.result

    def _ExtractHCode(self):
        with open(self.header_file_path, 'r') as f:
            lines = f.readlines()
            begin = None
            end = None
            for i, line in enumerate(lines):
                # TODO: Deduplicate those lines with ones in graphml_to_cpp.py
                if '//Start of h code from diagram' in line:
                    begin = i + 1
                if '//End of h code from diagram' in line:
                    end = i
            self.result.raw_h_code = ''.join(lines[begin:end]) if (begin and end) else ''


class StateParser:
    def __init__(self, ctx: ParsingContext, root_node):
        self.ctx = ctx
        self.root_node = root_node
        self.result = State(state_name=self.ctx.RemoveStateMachinNamePrefix(
            root_node.spelling))

    def Parse(self):
        self._TraverseAST(self.root_node)
        return self.result

    def _TraverseAST(self, node):
        if node.kind == clang.cindex.CursorKind.CASE_STMT:
            h = EventHandlerParser(self.ctx, node, self.result.state_name).Parse()
#            print('''EventHandler:
# Event: %s, direction: %s --> %s, Code:
# %s
#            ''' % (h.event_type, h.state_name, h.target_state_name, '\n'.join(h.statements)))
            self.result.event_handlers.extend(h.handlers)

        if node.kind == clang.cindex.CursorKind.DEFAULT_STMT:
            # That's how subtree of such node is expected to look like (generated by 'clang -Xclang -ast-dump <filename>.cpp')
            # `-DefaultStmt 0x7ffff3a84a08 <line:599:9, line:602:9>
            #     `-CompoundStmt 0x7ffff3a849e8 <line:599:18, line:602:9>
            #     |-BinaryOperator 0x7ffff3a849b8 <line:600:13, ./qhsm.h:72:26> 'QState':'int' lvalue '='
            #     | |-DeclRefExpr 0x7ffff3a84628 <oregonPlayer.cpp:600:13> 'QState':'int' lvalue Var 0x7ffff3a82dd0 'status_' 'QState':'int'
            #     | `-ParenExpr 0x7ffff3a84998 <./qhsm.h:71:5, line:72:26> 'QState':'int'
            #     |   `-BinaryOperator 0x7ffff3a84970 <line:71:6, line:72:25> 'QState':'int' ','
            #     |     |-BinaryOperator 0x7ffff3a84880 <line:71:6, line:63:56> 'QStateHandler':'QState (*)(void *const, const QEvt *const)' lvalue '='
            #     |     | |-MemberExpr 0x7ffff3a84758 <line:71:6, col:26> 'QStateHandler':'QState (*)(void *const, const QEvt *const)' lvalue ->effective_ 0x7ffff3a1b3a8
            #     |     | | `-ParenExpr 0x7ffff3a84738 <col:6, col:23> 'QHsm *'
            #     |     | |   `-ParenExpr 0x7ffff3a84718 <line:62:26, col:39> 'QHsm *'
            #     |     | |     `-CStyleCastExpr 0x7ffff3a846f0 <col:27, col:38> 'QHsm *' <BitCast>
            #     |     | |       `-ImplicitCastExpr 0x7ffff3a846d8 <col:35, col:38> 'OregonPlayer *' <LValueToRValue>
            #     |     | |         `-ParenExpr 0x7ffff3a846b8 <col:35, col:38> 'OregonPlayer *const' lvalue
            #     |     | |           `-DeclRefExpr 0x7ffff3a84650 <line:71:20> 'OregonPlayer *const' lvalue ParmVar 0x7ffff3a82c10 'me' 'OregonPlayer *const'
            #     |     | `-ParenExpr 0x7ffff3a84860 <line:63:31, col:56> 'QStateHandler':'QState (*)(void *const, const QEvt *const)'
            #     |     |   `-CStyleCastExpr 0x7ffff3a84838 <col:32, col:55> 'QStateHandler':'QState (*)(void *const, const QEvt *const)' <BitCast>
            #     |     |     `-ParenExpr 0x7ffff3a84818 <col:47, col:55> 'QState (*)(OregonPlayer *const, const QEvt *const)'
            #     |     |       `-UnaryOperator 0x7ffff3a847b8 <oregonPlayer.cpp:600:31, col:32> 'QState (*)(OregonPlayer *const, const QEvt *const)' prefix '&'
            #     |     |         `-DeclRefExpr 0x7ffff3a84790 <col:32> 'QState (OregonPlayer *const, const QEvt *const)' lvalue Function 0x7ffff3a692b0 'OregonPlayer_global' 'QState (OregonPlayer *const, const QEvt *const)'
            #     |     `-CStyleCastExpr 0x7ffff3a84948 <./qhsm.h:72:5, col:25> 'QState':'int' <NoOp>
            #     |       `-ImplicitCastExpr 0x7ffff3a84930 <col:13, col:25> 'QState':'int' <IntegralCast>
            #     |         `-ParenExpr 0x7ffff3a84910 <col:13, col:25> '(anonymous enum at ./qhsm.h:46:1)'
            #     |           `-DeclRefExpr 0x7ffff3a848a8 <col:14> '(anonymous enum at ./qhsm.h:46:1)' EnumConstant 0x7ffff3a1b090 'Q_RET_SUPER' '(anonymous enum at ./qhsm.h:46:1)
            #     `-BreakStmt 0x7ffff3a849e0 <oregonPlayer.cpp:601:13>
            compound_stmt = list(node.get_children())[0]
            assert compound_stmt.kind == clang.cindex.CursorKind.COMPOUND_STMT, self.ctx.GetNodeText(
                compound_stmt)
            binary_op = list(compound_stmt.get_children())[0]
            assert binary_op.kind == clang.cindex.CursorKind.BINARY_OPERATOR, self.ctx.GetNodeText(
                binary_op)
            rhs = list(binary_op.get_children())[1]
            assert rhs.kind == clang.cindex.CursorKind.PAREN_EXPR, self.ctx.GetNodeText(
                rhs)
            # GetText(rhs) should look like Q_SUPER(&OregonPlayer_global);
            self.result.parent_state_name = self.ctx.GetNodeText(
                rhs)[len('Q_SUPER(&'):-len(');')]
            if self.result.parent_state_name == 'QHsm_top':
                self.result.parent_state_name = None
            else:
                self.result.parent_state_name = self.ctx.RemoveStateMachinNamePrefix(
                    self.result.parent_state_name)

        for childNode in node.get_children():
            self._TraverseAST(childNode)


class EventHandlerParser:
    handlers: List[EventHandler]
    root_node: clang.cindex.Cursor
    event_type: str
    state_from: str

    def __init__(self, ctx: ParsingContext, root_node, state_from):
        self.ctx = ctx
        assert root_node.kind == clang.cindex.CursorKind.CASE_STMT, root_node.kind
        # That's how subtree of such node is expected to look like (generated by 'clang -Xclang -ast-dump <filename>.cpp')
        # |-CaseStmt 0x7ffff3a83de8 <line:589:9, line:593:9>
        # | |-ImplicitCastExpr 0x7ffff3a84a80 <line:589:14> 'QSignal':'int' <IntegralCast>
        # | | `-DeclRefExpr 0x7ffff3a83dc0 <col:14> 'PlayerSignals' EnumConstant 0x7ffff3a28b28 'PILL_RESET_SIG' 'PlayerSignals'
        # | `-CompoundStmt 0x7ffff3a84318 <col:30, line:593:9>
        children = list(root_node.get_children())
        self.handlers = []
        self.event_type = list(children[0].get_children())[0].spelling
        self.root_node = children[1]
        assert self.root_node.kind == clang.cindex.CursorKind.COMPOUND_STMT, self.root_node.kind
        self.state_from = state_from

    def Parse(self):
        self.handlers = self._HandlersFromCompountStmt(self.root_node)
        return self

    def _HandlersFromCompountStmt(self, compount_stmt_node):
        assert compount_stmt_node.kind == clang.cindex.CursorKind.COMPOUND_STMT, compount_stmt_node.kind
        statements = []
        condition = None
        state_to = None

        for node in compount_stmt_node.get_children():
            # There is no support for other statements / flow control (e.g. for (...)).
            # So let's fail fast if something unexpected was encountered.
            assert (node.kind == clang.cindex.CursorKind.CALL_EXPR or
                    node.kind == clang.cindex.CursorKind.BINARY_OPERATOR or
                    node.kind == clang.cindex.CursorKind.UNARY_OPERATOR or
                    node.kind == clang.cindex.CursorKind.BREAK_STMT or
                    node.kind == clang.cindex.CursorKind.IF_STMT), self.ctx.GetNodeText(node)

            if node.kind == clang.cindex.CursorKind.IF_STMT:
                children = list(node.get_children())
                assert len(children) == 3
                condition, if_branch, else_branch = children
                if_handler = self._HandlersFromCompountStmt(if_branch)[0]
                if_handler.condition = self.ctx.GetNodeText(condition)[:-1] # cut out last ')'
                else_handler = self._HandlersFromCompountStmt(else_branch)[0]
                else_handler.condition = 'else'
                return [if_handler, else_handler]

            if node.kind == clang.cindex.CursorKind.BREAK_STMT:
                return [EventHandler(state_from=self.state_from, state_to=state_to, condition=condition, statements=statements)]

            # It's a bit hacky:
            # 1) BINARY_OPERATOR doesn't necessarily mean equality operator. Can be some weird dangling statement like 'a < Foo(5);'
            # 2) It would be better to check if it is a status_ assignment by checking first child, which should be DECL_REF_EXPR node
            if node.kind == clang.cindex.CursorKind.BINARY_OPERATOR and self.ctx.GetNodeText(node).startswith('status_ = '):
                # That's how subtree of such node is expected to look like (generated by 'clang -Xclang -ast-dump <filename>.cpp')
                # |-BinaryOperator 0x7ffff3a845d0 <line:596:13, ./qhsm.h:66:45> 'QState':'int' lvalue '='
                # | |-DeclRefExpr 0x7ffff3a844c0 <oregonPlayer.cpp:596:13> 'QState':'int' lvalue Var 0x7ffff3a82dd0 'status_' 'QState':'int'
                # | `-ParenExpr 0x7ffff3a845b0 <./qhsm.h:66:21, col:45> 'QState':'int'
                # |   `-CStyleCastExpr 0x7ffff3a84588 <col:22, col:44> 'QState':'int' <NoOp>
                # |     `-ImplicitCastExpr 0x7ffff3a84570 <col:30, col:44> 'QState':'int' <IntegralCast>
                # |       `-ParenExpr 0x7ffff3a84550 <col:30, col:44> '(anonymous enum at ./qhsm.h:46:1)'
                # |         `-DeclRefExpr 0x7ffff3a844e8 <col:31> '(anonymous enum at ./qhsm.h:46:1)' EnumConstant 0x7ffff3a1b120 'Q_RET_HANDLED' '(anonymous enum at ./qhsm.h:46:1)'
                rhs = list(node.get_children())[1]
                if self.ctx.GetNodeText(rhs) == 'Q_HANDLED();':
                    state_to = self.state_from
                else:
                    # Cut state name from a string like 'Q_TRAN(&OregonPlayer_ghoul_wounded);'
                    state_to = self.ctx.RemoveStateMachinNamePrefix(
                        self.ctx.GetNodeText(rhs)[len('Q_TRAN(&'):-len(');')]
                    )
                return [EventHandler(state_from=self.state_from, state_to=state_to, condition=condition, statements=statements, event_type=self.event_type)]
            else:
                statements.append(self.ctx.GetNodeText(node))


class StateMachineWriter:
    state_name_to_node_name: Dict[str, str] = field(default_factory=dict)
    edge_id: int

    # TODO(aeremin) Pass some data object instead of whole StateMachineParser
    def __init__(self, state_machine: StateMachine):
        self.state_machine = state_machine

    def WriteToFile(self, filename: str, previous_filename: Optional[str] = None):
        previous_filename = previous_filename or filename
        previous_states = []
        if os.path.isfile(previous_filename):
            # previous_filename = os.path.splitext(previous_filename)[0]
            previous_states, minx, miny = graphmltoqm.get_states_from_graphml(previous_filename)

        graphml_root_node = create_graphml.prepare_graphml()
        self.graph = create_graphml.create_graph(graphml_root_node, 'G')
        create_graphml.add_start_state(self.graph, "n0")
        self.state_name_to_node_name = {}
        self.edge_id = 1

        child_index = 1
        for state in self.state_machine.states['global'].child_states:
            self._OutputState(state, child_index, self.graph, previous_states)
            child_index += 1

        create_graphml.add_edge(self.graph, "e0",
                                "n0", self.state_name_to_node_name[self.state_machine.initial_state],
                                self.state_machine.initial_code,
                                0, 0, 0, 0, [])

        for state_name in self.state_machine.states:
            state = self.state_machine.states[state_name]
            for h in state.event_handlers:
                if not h.state_to:
                    continue
                if h.state_from == h.state_to:
                    continue
                self._OutputEdge(h)

        create_graphml.get_constructor_code_comment(self.graph, 'constructor_code', self.state_machine.constructor_code)
        create_graphml.get_constructor_fields_comment(self.graph, 'constructor_fields', self.state_machine.constructor_fields)
        create_graphml.get_event_fields_comment(self.graph, 'event_fields', self.state_machine.event_fields)
        create_graphml.get_h_code_comment(self.graph, "raw_h_code", self.state_machine.raw_h_code)
        create_graphml.get_state_fields_comment(self.graph, "state_fields", self.state_machine.state_fields)

        create_graphml.finish_graphml(graphml_root_node)
        xml_tree = etree.ElementTree(graphml_root_node)
        xml_tree.write(
            filename, xml_declaration=True, encoding="UTF-8")

    def _OutputEdge(self, h: EventHandler):
        link_caption = h.event_type
        assert link_caption.endswith('_SIG')
        link_caption = link_caption[:-len('_SIG')]
        if h.condition:
            link_caption = link_caption + '[%s]' % h.condition

        if h.statements:
            link_caption = link_caption + '/' + '\n'.join(h.statements)
        create_graphml.add_edge(self.graph, "e%d" % self.edge_id,
                                self.state_name_to_node_name[h.state_from],
                                self.state_name_to_node_name[h.state_to],
                                link_caption,
                                0, 0, 0, 0, [])
        self.edge_id += 1

    def _OutputState(self, state, index_as_child, graph_parent, previous_states):
        state_content = list()
        for h in state.event_handlers:
            if h.state_from != h.state_to:
                continue
            event_type = h.event_type
            if event_type == 'Q_ENTRY_SIG':
                event_type = 'entry'
            elif event_type == 'Q_EXIT_SIG':
                event_type = 'exit'
            else:
                assert event_type.endswith('_SIG')
                event_type = event_type[:-len('_SIG')]

            state_content.append(
                ('%s' % event_type) + ('[%s]' % h.condition if h.condition else '') + '/')
            for statement in h.statements:
                for line in statement.split('\n'):
                    state_content.append('  ' + line)
            state_content.append('')

        full_node_name = (self.state_name_to_node_name[state.parent_state_name] + ':' if state.parent_state_name != 'global'
                          else '') + 'n%d' % index_as_child
        self.state_name_to_node_name[state.state_name] = full_node_name
        h = 100
        w = 200
        x0 = 259
        y0 = 255
        previous_states_with_name = [s for s in previous_states if s.name == state.state_name]
        if previous_states_with_name:
            print('For state %s using cached coordinates' % state.state_name)
            h = previous_states_with_name[0].height * qm.divider
            w = previous_states_with_name[0].width * qm.divider
            x0 = previous_states_with_name[0].x * qm.divider
            y0 = previous_states_with_name[0].y * qm.divider
        else:
            print('For state %s using DUMB coordinates' % state.state_name)

        if state.child_states:
            group_node = create_graphml.add_group_node(graph_parent, state.state_name, '\n'.join(
                state_content), full_node_name, h, w, x0, y0)
            parent = create_graphml.create_graph(
                group_node, full_node_name + ':')

            child_index = 0
            for child_state in state.child_states:
                self._OutputState(child_state, child_index, parent, previous_states)
                child_index += 1
        else:
            create_graphml.add_simple_node(graph_parent, state.state_name, '\n'.join(
                state_content), full_node_name, h, w, x0, y0)


if __name__ == '__main__':
    file_path = sys.argv[1]
    assert file_path.endswith(
        '.cpp'), 'First command line argument should be a *.cpp file!'
    parser = StateMachineParser(file_path)
    StateMachineWriter(parser.Parse()).WriteToFile(
        file_path.replace('.cpp', '.graphml'), file_path.replace('.cpp', '1.graphml'))
