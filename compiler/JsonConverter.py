"""Legacy module implements Lapki IDE's internal scheme to yed-GraphMl."""
from typing import Any, Dict, List
from copy import deepcopy

import xmltodict
from aiohttp.web import WebSocketResponse
from compiler.types.ide_types import InitialState, Point
from compiler.fullgraphmlparser.stateclasses import ParserState
from compiler.Logger import Logger


DEFAULT_TRANSITION_DATA = {
    '@key': 'd10',
    'y:PolyLineEdge': {
        'y:Path': {
            '@sx': '0',
            '@sy': '0',
            '@tx': '0',
            '@ty': '0'
        },
        'y:LineStyle': {
            '@color': '#000000',
            '@type': 'line',
            '@width': '1.0'
        },
        'y:Arrows': {
            '@source': 'none',
            '@target': 'standard'
        },
        'y:EdgeLabel': {
            '#text': '',
            '@alignment': 'center',
            '@backgroundColor': '#F5F5F5',
            '@configuration': 'AutoFlippingLabel',
            '@distance': '2.0',
            '@fontFamily': 'Dialog',
            '@fontSize': '12',
            '@fontStyle': 'plain',
            '@hasLineColor': 'false',
            '@horizontalTextPosition': 'center',
            '@iconTextGap': '4',
            '@modelName': 'centered',
            '@modelPosition': 'center',
            '@preferredPlacement': 'center_on_edge',
            '@ratio': '5.0',
            '@textColor': '#000000',
            '@verticalTextPosition': 'bottom',
            '@visible': 'false',
            '@xml:space': 'preserve',
        }
    }
}


class JsonConverter:
    """Класс для экспорта в берлогу."""

    def __init__(self, ws: WebSocketResponse) -> None:
        self.ws = ws
        self.transitions: List[Dict[str, Any]] = []

    def _getEvents(self, state: ParserState) -> str:
        """
        Функция формирует события в состоянии и действия\
            при их наступлении.

        Также формирует список переходов self.transitions.

        Вход: ParserState
        Пример возвращаемого значения:
            entry/
            ОружиеЦелевое.АтаковатьЦель();

            exit/
        """
        events: list[str] = []
        events.append('\n'.join(['entry/', state.entry]))
        for trig in state.trigs:
            trig.name = trig.name.replace('_', '.')
            if trig.type == 'internal':
                event = '\n'.join([f'{trig.name}/', f'{trig.action}'])

                events.append(event)
            else:
                transition: Dict[str, Any] = {
                    '@source': trig.source,
                    '@target': trig.target,
                    'data': deepcopy(DEFAULT_TRANSITION_DATA)
                }
                if trig.guard == 'true':
                    transition['data']['y:PolyLineEdge']['y:EdgeLabel'][
                        '#text'] = f'{trig.name}/{trig.action}\n'
                else:
                    transition['data']['y:PolyLineEdge']['y:EdgeLabel'][
                        '#text'] = f'{trig.name}/\n\
                            [{trig.guard}]\n{trig.action}'
                self.transitions.append(transition)
        events.append('\n'.join(['exit/', state.exit]))
        return ''.join(events)

    def _recursiveGetStates(self,
                            state: ParserState,
                            graph: Dict[str, Any]) -> Dict[str, Any]:
        xmlstate: Dict[str, Any] = {
            '@id': f'{state.id}',
            'data': [
                {
                    '@key': 'd6'
                }
            ]
        }

        if len(state.childs) > 0:
            if state.name != 'global':
                node_type = 'y:GroupNode'
                xmlstate['data'].insert(0, {
                    '@key': 'd4',
                    '@xml:space': 'preserve',
                })

                xmlstate['@yfiles.foldertype'] = 'group'
                xmlstate['data'][1]['y:ProxyAutoBoundsNode'] = {
                    'y:Realizers': {
                        '@active': '0',
                        node_type: {
                            'y:NodeLabel': [
                                {
                                    '@alignment': 'center',
                                    '@autoSizePolicy': 'node_width',
                                    '@backgroundColor': '#B7C9E3',
                                    '@fontFamily': 'Dialog',
                                    '@fontSize': '15',
                                    '@fontStyle': 'bold',
                                    '@hasLineColor': 'false',
                                    '@height': '22',
                                    '@horizontalTextPosition': 'center',
                                    '@iconTextGap': '4',
                                    '@modelName': 'internal',
                                    '@modelPosition': 't',
                                    '@textColor': '#000000',
                                    '@verticalTextPosition': 'bottom',
                                    '@visible': 'true',
                                    '@width': state.bounds.width,
                                    '@x': '0',
                                    '@xml:space': 'preserve',
                                    '@y': '0',
                                    '#text': f'{state.name}'
                                },
                                {
                                    '@alignment': 'left',
                                    '@autoSizePolicy': 'node_size',
                                    '@backgroundColor': '#B7C9E3',
                                    '@fontFamily': 'Consolas',
                                    '@fontSize': '15',
                                    '@fontStyle': 'Plain',
                                    '@hasLineColor': 'false',
                                    '@height': '22',
                                    '@horizontalTextPosition': 'center',
                                    '@iconTextGap': '4',
                                    '@modelName': 'custom',
                                    '@modelPosition': 't',
                                    '@textColor': '#000000',
                                    '@verticalTextPosition': 'bottom',
                                    '@visible': 'true',
                                    '@width': state.bounds.width,
                                    '@x': '0',
                                    '@xml:space': 'preserve',
                                    '@y': '0',
                                    '#text': self._getEvents(state)
                                },
                            ],
                            'y:Geometry': {
                                '@x': state.bounds.x,
                                '@y': state.bounds.y,
                                '@width': state.bounds.width,
                                '@height': state.bounds.height
                            },
                            'y:Shape': {
                                '@type': 'roundrectangle'
                            },
                            'y:ParserState': {
                                '@closed': 'false',
                                '@closedHeight': '50.0',
                                '@closedWidth': '50.0',
                                '@innerGraphDisplayEnabled': 'false'
                            },
                            'y:NodeBounds': {
                                '@considerNodeLabelSize': 'true'
                            },
                            'y:Insets': {
                                '@bottom': '15',
                                '@bottomF': '15.0',
                                '@left': '15',
                                '@leftF': '15.0',
                                '@right': '15',
                                '@rightF': '15.0',
                                '@top': '15',
                                '@topF': '15.0'
                            },
                            'y:BorderInsets': {
                                '@bottom': '0',
                                '@bottomF': '0.0',
                                '@left': '0',
                                '@leftF': '0.0',
                                '@right': '0',
                                '@rightF': '0.0',
                                '@top': '0',
                                '@topF': '0.0'
                            }
                        }
                    },
                }

                xmlstate['graph'] = {
                    '@edgedefault': 'directed',
                    '@id': f':{state.id}'
                }

                xmlstate['graph']['node'] = []  # type: ignore
                for child in state.childs:
                    xmlstate['graph']['node'].append(
                        self._recursiveGetStates(child, xmlstate['graph']))
            else:
                for child in state.childs:
                    graph['node'].append(self._recursiveGetStates(
                        child,
                        graph
                    )
                    )

        else:
            node_type = 'y:GenericNode'

            if state.parent:
                state.bounds.x += state.parent.bounds.x
                state.bounds.y += state.parent.bounds.y

            xmlstate['data'][0][node_type] = {
                'y:NodeLabel': [
                    {
                        '@alignment': 'center',
                        '@autoSizePolicy': 'node_width',
                        '@backgroundColor': '#B7C9E3',
                        '@configuration': 'com.yworks.\
                            entityRelationship.label.name',
                        '@fontFamily': 'Dialog',
                        '@fontSize': '15',
                        '@fontStyle': 'bold',
                        '@hasLineColor': 'false',
                        '@height': '22',
                        '@horizontalTextPosition': 'center',
                        '@iconTextGap': '4',
                        '@modelName': 'internal',
                        '@modelPosition': 't',
                        '@textColor': '#000000',
                        '@verticalTextPosition': 'bottom',
                        '@visible': 'true',
                        '@width': state.bounds.width,
                        '@x': '0',
                        '@xml:space': 'preserve',
                        '@y': '0',
                        '#text': f'{state.name}'
                    },
                    {
                        '@alignment': 'left',
                        '@autoSizePolicy': 'content',
                        '@backgroundColor': '#B7C9E3',
                        '@configuration': 'com.yworks.\
                            entityRelationship.label.name',
                        '@fontFamily': 'Consolas',
                        '@fontSize': '15',
                        '@fontStyle': 'Plain',
                        '@hasLineColor': 'false',
                        '@height': state.bounds.height,
                        '@horizontalTextPosition': 'center',
                        '@iconTextGap': '4',
                        '@modelName': 'custom',
                        '@modelPosition': 't',
                        '@textColor': '#000000',
                        '@verticalTextPosition': 'bottom',
                        '@visible': 'true',
                        '@width': state.bounds.width,
                        '@x': '0',
                        '@xml:space': 'preserve',
                        '@y': '0',
                        '#text': self._getEvents(state)
                    },
                ],
                'y:Geometry': {
                    '@x': state.bounds.x,
                    '@y': state.bounds.y,
                    '@width': state.bounds.width,
                    '@height': state.bounds.height
                },
                'y:Fill': {
                    '@color': '#E8EEF7',
                    '@color2': '#B7C9E3',
                    '@transparent': 'false'
                },
                'y:BorderStyle': {
                    '@color': '#000000',
                    '@type': 'line',
                    '@width': '1.0'
                },
            }

        return xmlstate

    def _getStates(self, states: List[ParserState]) -> Dict[str, Any]:
        graph: Dict[str, Any] = {
            '@edgedefault': 'directed',
            '@id': 'G',
            'data': {
                '@key': 'd0',
                '@xml:space': 'preserve',
            },
            'node': [
                {
                    '@id': '',
                    'data': {
                        '@key': 'd6',
                        'y:GenericNode': {
                            '@configuration': ('com.yworks.bpmn.'
                                               'Event.withShadow'),
                            'y:NodeLabel': {},
                            'y:Fill': {
                                '@color': '#333333',
                                '@color2': '#000000',
                                '@transparent': 'false'
                            },
                            'y:BorderStyle': {
                                '@color': '#000000',
                                '@type': 'line',
                                '@width': '1.0'
                            }
                        },
                        'y:Geometry': {
                            '@x': 0,
                            '@y': 0,
                            '@width': 10,
                            '@height': 10
                        }
                    }
                }
            ]
        }
        graph['node'].append(self._recursiveGetStates(
            states[0],
            graph=graph))
        graph['node'] = graph['node'][:-1]

        return graph

    def _addInitialState(self, initial_state: str):
        self.transitions.append(
            {
                '@source': '',
                '@target': initial_state,
                'data': DEFAULT_TRANSITION_DATA
            }
        )

    def _addNodePreferredToEdgeLabels(self, xml: str) -> str:
        unprocessed = xml
        label = '</y:EdgeLabel>'
        edge_label_pos = unprocessed.find(label)
        temp = ''
        preffered = '\n<y:PreferredPlacementDescriptor \
            \ngle="0.0" angleOffsetOnRightSide="0"\
            angleReference="absolute" angleRotationOnRightSide="co" \
                distance="-1.0" placement="center" side="on_edge"\
                    sideReference="relative_to_edge_flow" />\n'
        # last_position = -1
        while (edge_label_pos != -1):
            temp += unprocessed[:edge_label_pos] + preffered + \
                unprocessed[edge_label_pos: edge_label_pos + len(label)]
            unprocessed = unprocessed[edge_label_pos + len(label):]
            # last_position = edge_label_pos
            edge_label_pos = unprocessed.find(label)

        temp += unprocessed

        return temp

    async def parse(
        self,
        states: Dict[str, ParserState],
        initial_state: InitialState
    ) -> str:
        """Convert states to yed-Graphml."""
        try:
            self.states = states
            init_position: Point = initial_state.position
            init_pos_x = str(init_position.x).replace('.', ',')
            init_pos_y = str(init_position.y).replace('.', ',')
            data: Dict[str, Any] = {
                'graphml': {
                    '@xmlns': 'http://graphml.graphdrawing.org/xmlns',
                    '@xmlns:java': 'http://www.yworks.com/xml/\
                        yfiles-common/1.0/java',
                    '@xmlns:sys': 'http://www.yworks.com/xml/yfiles-common/\
                        markup/primitives/2.0',
                    '@xmlns:x': 'http://www.yworks.com/xml/\
                        yfiles-common/markup/2.0',
                    '@xmlns:xsi': 'http://www.w3.org/2001/XMLSchema-instance',
                    '@xmlns:y': 'http://www.yworks.com/xml/graphml',
                    '@xmlns:yed': 'http://www.yworks.com/xml/yed/3',
                    '@yed:schemaLocation': 'http://graphml.graphdrawing.org/\
                        xmlns http://www.yworks.com/xml/schema/graphml/1.1/\
                            ygraphml.xsd',
                    '@entryPosition': f'{init_pos_x} {init_pos_y}',
                    'key': [
                        {
                            '@attr.name': 'Description',
                            '@attr.type': 'string',
                            '@for': 'graph',
                            '@id': 'd0'
                        },
                        {
                            '@for': 'port',
                            '@id': 'd1',
                            '@yfiles.type': 'portgraphics'
                        },
                        {
                            '@for': 'port',
                            '@id': 'd2',
                            '@yfiles.type': 'portgeometry'
                        },
                        {
                            '@for': 'port',
                            '@id': 'd3',
                            '@yfiles.type': 'portuserdata'
                        },
                        {
                            '@attr.name': 'url',
                            '@attr.type': 'string',
                            '@for': 'node',
                            '@id': 'd4'
                        },
                        {
                            '@attr.name': 'description',
                            '@attr.type': 'string',
                            '@for': 'node',
                            '@id': 'd5'
                        },
                        {
                            '@for': 'node',
                            '@id': 'd6',
                            '@yfiles.type': 'nodegraphics'
                        },
                        {
                            '@for': 'graphml',
                            '@id': 'd7',
                            '@yfiles.type': 'resources'
                        },
                        {
                            '@attr.name': 'url',
                            '@attr.type': 'string',
                            '@for': 'edge',
                            '@id': 'd8'
                        },
                        {
                            '@attr.name': 'description',
                            '@attr.type': 'string',
                            '@for': 'edge',
                            '@id': 'd9'
                        },
                        {
                            '@for': 'edge',
                            '@id': 'd10',
                            '@yfiles.type': 'edgegraphics'
                        }
                    ],
                    'graph': self._getStates(list(states.values())),
                }
            }

            self._addInitialState(initial_state.target)
            data['graphml']['graph']['edge'] = self.transitions
            result = self._addNodePreferredToEdgeLabels(
                xmltodict.unparse(data, pretty=True))

            return result
        except Exception:
            await Logger.logException()
            return ''
