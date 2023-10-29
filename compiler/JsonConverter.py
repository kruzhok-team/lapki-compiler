import xmltodict

try:
    from compiler.fullgraphmlparser.stateclasses import State
except ImportError:
    from .fullgraphmlparser.stateclasses import State


class JsonConverter:
    """
        Класс для экспорта в берлогу.
    """

    def __init__(self, ws) -> None:
        self.ws = ws
        self.transitions: list[dict] = []

    async def getEvents(self, state: State) -> str:
        """
            Функция формирует события в состоянии и действия при их наступлении.
            Также формирует список переходов self.transitions.

            Вход: State
            Пример возвращаемого значения:
                entry/
                ОружиеЦелевое.АтаковатьЦель();

                exit/
        """
        events: list[str] = []
        events.append("\n".join(["entry/", state.entry]))
        for trig in state.trigs:
            trig.name = trig.name.replace('_', '.')
            if trig.type == "internal":
                event = "\n".join([f"{trig.name}/", f"{trig.action}"])

                events.append(event)
            else:
                if trig.guard == "true":
                    transition = {
                        "@source": trig.source,
                        "@target": trig.target,
                        "y:EdgeLabel": f"{trig.name}/{trig.action}\n"
                    }
                else:
                    transition = {
                        "@source": trig.source,
                        "@target": trig.target,
                        "y:EdgeLabel": f"{trig.name}/\n[{trig.guard}]\n{trig.action}"
                    }
                self.transitions.append(transition)
        events.append("\n".join(["exit/", state.exit]))
        return "".join(events)

    async def _recursiveGetStates(self, state: State, graph: dict, parent: str = '') -> dict:
        xmlstate = {
            "@id": f"{state.id}",
            "data": [
                {
                    "@key": "d6"
                }
            ]
        }

        if len(state.childs) > 0:
            if state.name != "global":
                node_type = "y:GroupNode"
                xmlstate["data"].insert(0, {
                    "@key": "d4",
                    "@xml:space": "preserve"
                })
                xmlstate["@yfiles:foldertype"] = "group"
                xmlstate["data"][1][node_type] = {
                    "y:NodeLabel": [
                        {
                            "@alignment": "center",
                            "@autoSizePolicy": "node_width",
                            "@backgroundColor": "#B7C9E3",
                            "@configuration": "com.yworks.entityRelationship.label.name",
                            "@fontFamily": "Dialog",
                            "@fontSize": "15",
                            "@fontStyle": "bold",
                            "@hasLineColor": "false",
                            "@height": "22",
                            "@horizontalTextPosition": "center",
                            "@iconTextGap": "4",
                            "@modelName": "internal",
                            "@modelPosition": "t",
                            "@textColor": "#000000",
                            "@verticalTextPosition": "bottom",
                            "@visible": "true",
                            "@width": state.width,
                            "@x": "0",
                            "@xml:space": "preserve",
                            "@y": "0",
                            "#text": f"{state.name}"
                        },
                        {
                            "@alignment": "left",
                            "@autoSizePolicy": "node_width",
                            "@backgroundColor": "#B7C9E3",
                            "@configuration": "com.yworks.entityRelationship.label.name",
                            "@fontFamily": "Dialog",
                            "@fontSize": "15",
                            "@fontStyle": "bold",
                            "@hasLineColor": "false",
                            "@height": "22",
                            "@horizontalTextPosition": "center",
                            "@iconTextGap": "4",
                            "@modelName": "internal",
                            "@modelPosition": "t",
                            "@textColor": "#000000",
                            "@verticalTextPosition": "bottom",
                            "@visible": "true",
                            "@width": state.width,
                            "@x": "0",
                            "@xml:space": "preserve",
                            "@y": "0",
                            "#text": await self.getEvents(state)
                        }
                    ],
                    "y:Geometry": {
                        "@x": state.x,
                        "@y": state.y,
                        "@width": state.width,
                        "@height": state.height
                    }
                }

                xmlstate["graph"] = {
                    "@edgedefault": "directed",
                    "@id": f":{state.id}"
                }
                xmlstate["graph"]["node"] = []
                for child in state.childs:
                    xmlstate["graph"]["node"].append(await self._recursiveGetStates(child, xmlstate["graph"], parent=f"{state.id}::"))
            else:
                for child in state.childs:
                    graph["node"].append(await self._recursiveGetStates(child, graph))

        else:
            node_type = "y:GenericNode"

            if state.parent:
                state.x += state.parent.x
                state.y += state.parent.y

            # xmlstate["data"].append({
            #     "@key": "d6"
            # })
            #

            xmlstate["data"][0][node_type] = {
                "y:Fill": {
                    "@color": "#EBEEF7",
                    "@color2": "#B7C9E3",
                    "@transparent": "false",
                },
                "y:BorderStyle": {
                    "@color": "#000000",
                    "@type": "line",
                    "@width": "1.0",
                },
                "y:NodeLabel": [
                    {
                        "@alignment": "center",
                        "@autoSizePolicy": "node_width",
                        "@backgroundColor": "#B7C9E3",
                        "@configuration": "com.yworks.entityRelationship.label.name",
                        "@fontFamily": "Dialog",
                        "@fontSize": "15",
                        "@fontStyle": "bold",
                        "@hasLineColor": "false",
                        "@height": "22",
                        "@horizontalTextPosition": "center",
                        "@iconTextGap": "4",
                        "@modelName": "internal",
                        "@modelPosition": "t",
                        "@textColor": "#000000",
                        "@verticalTextPosition": "bottom",
                        "@visible": "true",
                        "@width": state.width,
                        "@x": "0",
                        "@xml:space": "preserve",
                        "@y": "0",
                        "#text": f"{state.name}"
                    },
                    await self.getEvents(state)
                ],
                "y:Geometry": {
                    "@x": state.x,
                    "@y": state.y,
                    "@width": state.width,
                    "@height": state.height
                }
            }

        return xmlstate

    async def getStates(self, states: list[State]) -> dict:
        graph = {
            "@edgedefault": "directed",
            "@id": "G",
            "data": {
                "@key": "d0",
                "@xml:space": "preserve",
            },
            "node": [
                {
                    "@id": "",
                    "data": {
                        "@key": "d6",
                        "y:GenericNode": {
                            "@configuration": "com.yworks.bpmn.Event.withShadow",
                            "y:NodeLabel": {},
                            "y:Fill": {
                                "@color": "#333333",
                                "@color2": "#000000",
                                "@transparent": "false"
                            },
                            "y:BorderStyle": {
                                "@color": "#000000",
                                "@type": "line",
                                "@width": "1.o"
                            }
                        }
                    }
                }
            ]
        }

        graph["node"].append(await self._recursiveGetStates(states[0], graph=graph))
        # TODO: Придумать как исправить костыль для удаления глобального состояния
        graph["node"] = graph["node"][:-1]

        return graph

    async def addInitialState(self, initial_state: str):
        self.transitions.append(
            {
                "@source": "",
                "@target": initial_state,
                "y:EdgeLabel": ""
            }
        )

    async def parse(self, states: dict[str, State], initial_state: str) -> str:
        self.states = states
        data = {"graphml": {
            "@xmlns": "http://graphml.graphdrawing.org/xmlns",
            "@xmlns:y": "http://www.yworks.com/xml/graphml",
            "key": [
                {
                    "@atrr.name": "Description",
                    "@attr.type": "string",
                    "@for": "graph",
                    "@id": "d0",
                },
                {
                    "@for": "port",
                    "@id": "d1",
                    "@yfiles.type": "portgraphics",
                },
                {
                    "@for": "port",
                    "@id": "d2",
                    "@yfiles.type": "portgeometry",
                },
                {
                    "@for": "port",
                    "@id": "d3",
                    "@yfiles.type": "portuserdata",
                },
                {
                    "@attr.name": "url",
                    "@attr.type": "string",
                    "@for": "node",
                },
                {
                    "@attr.name": "description",
                    "@attr.type": "string",
                    "@for": "node",
                    "@id": "d5",
                },
                {
                    "@for": "node",
                    "@id": "d6",
                    "@yfiles.type": "nodegraphics",
                },
                {
                    "@for": "graphml",
                    "@id": "d7",
                    "@yfiles.type": "resources"
                },
                {
                    "@attr.name": "url",
                    "@attr.type": "string",
                    "@for": "edge",
                    "@id": "d8"
                },
                {
                    "@attr.name": "description",
                    "@atrr.type": "string",
                    "@for": "edge",
                    "@id": "d9"
                },
                {
                    "@for": "edge",
                    "@id": "d10",
                    "@yfiles.type": "edgegraphics"
                }
            ],
            "graph": await self.getStates(list(states.values())),
        }
        }

        await self.addInitialState(initial_state)
        data["graphml"]["graph"]["edge"] = self.transitions
        result = xmltodict.unparse(data, pretty=True)

        return result
