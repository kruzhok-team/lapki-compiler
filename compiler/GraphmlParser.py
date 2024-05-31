"""Module implements parsing yed-Graphml."""
import random
from typing import Any, Dict, List, TypeAlias, Literal
from collections import defaultdict

import xmltodict
from compiler.types.platform_types import Method, MethodParameter, Platform
from compiler.Logger import Logger
from compiler.PlatformManager import PlatformManager

Point: TypeAlias = dict[Literal['x', 'y'], int]
TRANSTIONS_DISTANCE = 75
DIFF_THRESHHOLD = 150


class GraphmlParser:
    """This class gets Berloga-graphml and \
        returns States, Components, Transitions."""

    def __init__(self, platform: str, ws):
        pass

    colors = ['#00a550', '#9999ff', '#faf74d', '#ffa500', '#5dded3']

    systemSignalsAlias = {
        'entry': 'onEnter',
        'exit': 'onExit'
    }

    operatorAlias = {
        '!=': 'notEquals',
        '==': 'equals',
        '>': 'greater',
        '<': 'less',
        '>=': 'greaterOrEqual',
        '<=': 'lessOrEqual'
    }

    @staticmethod
    async def _getArgs(component_name: str,
                       method_name: str,
                       args: list[str],
                       platform_id: str):
        """функция, которая формирует аргументы в виде\
            объекта с учетом контекста платформы."""
        platform_manager = PlatformManager()
        platform: Platform = await platform_manager.get_platform(
            platform_id, '1.0')
        method: Method = (
            platform.components[component_name].methods[method_name])
        params: List[MethodParameter] = method.parameters
        result: Dict[str, str] = {}
        for i in range(len(args)):
            # Можно сделать проверку значений и типов
            result[params[i].name] = args[i]
        return result

    @staticmethod
    def _getParentNode(group_node: dict) -> dict:
        """
        Здесь мы отделяем данные о супер-ноде от ее под-графа.

        Необходимое содержимое data: {y:GroupNode: {
        y:NodeLabel: [название состояния, ивенты],
        y:Geometry: {@y, @x, @width, @height} } }
        """
        data_node_with_info = ''
        for data in group_node['data']:
            if 'y:ProxyAutoBoundsNode' in data.keys():
                data_node_with_info = \
                    data['y:ProxyAutoBoundsNode']['y:Realizers']

        return {
            '@id': group_node['@id'],
            'data': data_node_with_info
        }

    @staticmethod
    def _randColor() -> str:
        """Gen hex-color."""
        def r() -> int:
            return random.randint(0, 255)
        return '#%02X%02X%02X' % (r(), r(), r())

    @staticmethod
    def _addStateToDict(
        state: dict,
        states_dict: dict,
        parent: str | None
    ) -> None:
        if 'y:GenericNode' in state['data']:
            node_type = 'y:GenericNode'
        else:
            node_type = 'y:GroupNode'
        states_dict[state['@id']] = {}
        states_dict[state['@id']]['type'] = node_type
        states_dict[state['@id']]['parent'] = parent
        if 'y:Geometry' in state['data'][node_type]:
            geometry = state['data'][node_type]['y:Geometry']
            states_dict[state['@id']]['geometry'] = {
                'x': int(float(geometry['@x'])),
                'y': int(float(geometry['@y'])),
                'width': int(float(geometry['@width'])),
                'height': int(float(geometry['@height']))
            }
        else:
            states_dict[state['@id']]['geometry'] = {
                'x': 0,
                'y': 0,
                'width': 0,
                'height': 0
            }
        try:
            text = state['data'][node_type]['y:NodeLabel'][0]['#text']
            states_dict[state['@id']]['name'] = text
        except TypeError:
            pass

    @staticmethod
    def _getFlattenStates(
        xml: list[dict],
        states: list,
        states_dict: dict[str, dict[str, str]],
        nparent: str | None = None
    ) -> tuple[list[dict[str, str | dict]], dict[str, dict[str, str]]]:
        """Из иерархической структуры графа создаем плоскую."""
        for node in xml:
            if 'graph' in node.keys():
                parent = GraphmlParser._getParentNode(node)
                states.append(parent)
                GraphmlParser._addStateToDict(
                    parent, states_dict, parent=nparent)
                nodes = node['graph']['node']
                if isinstance(nodes, dict):
                    nodes = [nodes]
                GraphmlParser._getFlattenStates(
                    nodes,
                    states,
                    states_dict,
                    nparent=parent['@id']
                )

            else:
                GraphmlParser._addStateToDict(
                    node, states_dict, parent=nparent)
                states.append(node)
        return states, states_dict

    @staticmethod
    async def _getEvents(state: Dict[str, Any],
                         node_type: str,
                         platform: str
                         ) -> List[Dict[str, Dict[str, Any]]]:
        str_events: str = state['data'][node_type]['y:NodeLabel'][1]['#text']
        events: list[str] = str_events.split('\n')
        new_events: list[dict[str, List[Dict[str, Any]] | Dict[str, str]]] = []
        current_event: str = ''
        i = 0
        current_dict = []
        for ev in events:
            if '/' in ev:
                if '.' in ev:
                    command = ev.split('.')
                    component = command[0]
                    method = command[1][:-1]
                    new_events.append({
                        'trigger': {
                            'component': component,
                            'method': method,
                        },
                        'do': []
                    })
                    current_dict = new_events[i]['do']
                else:
                    current_event = ev[:-1].replace(' ', '')
                    signal = GraphmlParser.systemSignalsAlias[current_event]
                    new_events.append({
                        'trigger': {
                            'component': 'System',
                            'method': signal,
                        },
                        'do': []
                    })
                    current_dict = new_events[i]['do']
                i += 1
                continue
            action_dict = {}
            action = ev.split('.')
            component = action[0]
            action_dict['component'] = component
            bracket_pos = action[1].find('(')
            method = action[1][:bracket_pos]
            action_dict['method'] = method
            if bracket_pos != -1:
                args = action[1][bracket_pos + 1:-1].split(',')
                if args != ['']:
                    action_dict['args'] = await GraphmlParser._getArgs(
                        component, method, args, platform)
                else:
                    action_dict['args'] = {}
            current_dict.append(action_dict)  # type: ignore
        return new_events  # type: ignore

    @staticmethod
    def _getParentName(state: dict, states_dict: dict) -> str | None:
        id: str = state['@id']
        return states_dict[id]['parent']

    @staticmethod
    def _checkValueType(value: str) -> dict:
        if '.' in value:
            command = value.split('.')
            component = command[0]
            method = command[1]
            return {'type': 'component',
                    'value': {
                        'component': component,
                        'method': method,
                        'args': {}
                    }}
        else:
            return {'type': 'value',
                    'value': value}

    @staticmethod
    def _getCondition(condition: str) -> dict | None:
        result = None
        if condition != '':
            condition = condition.replace('[', '').replace(']', '')
            condition_list = condition.split()
            lval = GraphmlParser._checkValueType(condition_list[0])
            operator = condition_list[1]
            rval = GraphmlParser._checkValueType(condition_list[2])

            result = {
                'type': GraphmlParser.operatorAlias[operator],
                'value': [lval, rval]
            }
        return result

    @staticmethod
    def _calculateEdgePosition(
        count_actions: int,
        count_condtions: int,
        source_position: dict,
        target_position: dict,
        used_coordinates: defaultdict[tuple[float, float],
                                      Point]) -> dict[str, int]:
        x1, y1, w1, h1 = list(source_position.values())
        x2, y2, w2, h2 = list(target_position.values())
        nx: int = (x1 * 1.25 + x2) // 2 + \
            (100 * (1 + count_condtions + count_actions))
        ny: int = (y1 + y2) // 2 + \
            (100 * (1 + count_condtions + count_actions))
        for coord in list(used_coordinates.keys()):
            if (ny < coord[1] + TRANSTIONS_DISTANCE and
                    ny > coord[1] - TRANSTIONS_DISTANCE):
                if (nx < coord[0] + TRANSTIONS_DISTANCE and
                        nx > coord[1] - TRANSTIONS_DISTANCE):
                    nx = int(nx * (1 + 0.15 * used_coordinates[coord]
                                   ['x'])) + 130 * \
                        (used_coordinates[coord]['y'] - 1)
                    used_coordinates[coord]['x'] += 1
                ny = int(ny * (1 + 0.15 * used_coordinates[coord]['y'])
                         ) + 25 * (used_coordinates[coord]['x'] - 1)
                used_coordinates[coord]['y'] += 1
                break
            if (nx < coord[0] + TRANSTIONS_DISTANCE and
                    nx > coord[1] - TRANSTIONS_DISTANCE):
                if (ny < coord[1] + TRANSTIONS_DISTANCE and
                        ny > coord[1] - TRANSTIONS_DISTANCE):
                    ny = (int(
                        ny * (1 + 0.15 * used_coordinates[coord]['y'])) + 25 *
                        (used_coordinates[coord]['y'] - 1))
                    used_coordinates[coord]['y'] += 1
                nx = int(nx * (1 + 0.15 * used_coordinates[coord]
                         ['x'])) + 130 * (used_coordinates[coord]['x'] - 1)
                used_coordinates[coord]['x'] += 1
                break
        used_coordinates[(nx, ny)]['x'] += 1
        used_coordinates[(nx, ny)]['y'] += 1
        return {'x': nx, 'y': ny}

    @staticmethod
    async def _parseAction(action: str, platform) -> dict:
        component, method = action.split('.')
        call_pos = method.find('(')
        args = method[call_pos + 1:-1].split(',')
        method = method[:call_pos]
        if args == ['']:
            args = {}
        else:
            args = await GraphmlParser._getArgs(
                component,
                method,
                args,
                platform
            )
        return {
            'component': component,
            'method': method,
            'args': args
        }

    @staticmethod
    async def getActions(actions: list[str], platform) -> list[dict]:
        """Функция получает список действий и\
            возвращает их в нотации IDE Lapki.

        Args:
            actions (list[str]): список действий.
            Пример: ['Счётчик.Прибавить(231)', 'ОружиеЦелевое.АтаковатьЦель()']

        Returns:
            list[dict]: список словарей [{
                                        'component': Счётчик,
                                        'method': 'Прибавить',
                                        'args': ['231']
                                    }]
        """
        result: list[dict] = []
        for action in actions:
            result.append(await GraphmlParser._parseAction(action, platform))
        return result

    @staticmethod
    async def _getTransitions(
        triggers: list[dict],
        statesDict: dict,
        platform: str
    ) -> tuple[list, str]:
        transitions = []
        initial_state = ''
        used_coordinates = defaultdict(lambda: {'x': 1, 'y': 1})
        for trigger in triggers:
            transition = {}
            try:
                transition['source'] = trigger['@source']
                transition['target'] = trigger['@target']
                label = (trigger['data']['y:PolyLineEdge']
                         ['y:EdgeLabel']['#text'])
                # condition может содержать условие,
                # условия и действия, действия и пустую строку
                event, condition = label.split('/')
                t: list[str] = condition.strip().split('\n')
                if len(t) > 0 and t[0].startswith('['):
                    condition = t[0]
                    actions = t[1:]
                else:
                    condition = ''
                    if '' in t:
                        t.remove('')
                    actions = t
                actions = await GraphmlParser.getActions(actions, platform)
                component, method = event.split('.')
                transition['trigger'] = {
                    'component': component,
                    'method': method
                }
                transition['condition'] = GraphmlParser._getCondition(
                    condition
                )
                source_geometry = statesDict[trigger['@source']
                                             ]['new_geometry']
                target_geometry = statesDict[trigger['@target']
                                             ]['new_geometry']
                transition['position'] = GraphmlParser._calculateEdgePosition(
                    len(actions),
                    1 if condition else 0,
                    source_geometry,
                    target_geometry,
                    used_coordinates  # type: ignore
                )
                transition['do'] = actions
                transition['color'] = GraphmlParser._randColor()
                transitions.append(transition)
            except (AttributeError, KeyError):
                initial_state = trigger['@target']
        return transitions, initial_state

    @staticmethod
    def _getGeometry(id: str, states_dict: dict) -> dict:
        parent = states_dict[id]['parent']
        current_parent = parent
        p_x = 0
        p_y = 0

        if parent is not None:
            p_geometry = states_dict[current_parent]['geometry']
            p_x += p_geometry['x']
            p_y += p_geometry['y']
        geometry = states_dict[id]['geometry']
        h = geometry['height']
        w = geometry['width']
        x = geometry['x'] - p_x
        y = geometry['y'] - p_y
        if p_y != 0 and parent is not None:
            y -= 300
            if x < 0:
                x = 100
            if y > 0:
                y = -50  # TODO Зависимость от количества триггеров
        states_dict[id]['new_geometry'] = {
            'x': int(float(x)),
            'y': -int(float(y)),
            'width': int(float(w)),
            'height': int(float(h))
        }
        return {
            'x': int(float(x)),
            'y': -int(float(y)),
            'width': int(float(w)),
            'height': int(float(h))
        }

    @staticmethod
    async def _createStates(
            flattenStates: list[dict],
            states_dict: dict,
            platform: str) -> dict:
        states = {}
        for state in flattenStates:
            if state['@id'] == '':
                continue
            new_state = {}
            id = state['@id']
            node_type = states_dict[state['@id']]['type']
            new_state['name'] = states_dict[state['@id']]['name']
            new_state['events'] = await GraphmlParser._getEvents(
                state, node_type, platform)
            geometry = GraphmlParser._getGeometry(state['@id'], states_dict)
            new_state['bounds'] = geometry
            parent = GraphmlParser._getParentName(state, states_dict)
            if parent is not None and parent != '':
                new_state['parent'] = parent
            states[id] = new_state

        return states

    @staticmethod
    async def _getComponents(platform: str) -> dict:
        # Так как в схеме Берлоги нет компонентов,
        # достаем из платформы все компоненты
        result = {}
        platform_manager = PlatformManager()
        platform_object = await platform_manager.get_platform(platform, '1.0')
        if platform_object is not None:
            for component in platform_object.components:
                result[component] = {}
                result[component]['type'] = component
                result[component]['parameters'] = {}

        return result

    @staticmethod
    async def parse(unprocessed_xml: str, platform: str):
        """Parse yed-graphml text and returns Lapki IDE's internal scheme."""
        try:
            xml = xmltodict.parse(unprocessed_xml)
            Logger.logger.info(xml)
            graph = xml['graphml']['graph']
            nodes = graph['node']
            triggers = graph['edge']
            components = await GraphmlParser._getComponents(platform)
            flattenStates, states_dict = GraphmlParser._getFlattenStates(
                nodes, states=[], states_dict={})
            states = await GraphmlParser._createStates(
                flattenStates, states_dict, platform)
            transitions, initial_state = await GraphmlParser._getTransitions(
                triggers, states_dict, platform)
            obj_initial_state = states[initial_state]
            init_x = obj_initial_state['bounds']['x'] - 100
            init_y = obj_initial_state['bounds']['y'] - 100
            return {'states': states,
                    'initialState': {
                        'target': initial_state,
                        'position': {
                            'x': init_x,
                            'y': init_y
                        }
                    },
                    'transitions': transitions,
                    'components': components,
                    'platform': platform,
                    'parameters': {}}
        except Exception:
            await Logger.logException()
