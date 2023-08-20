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
            "data": {}
        }

        if len(state.childs) > 0:
            if state.name != "global":
                node_type = "y:GroupNode"
                xmlstate["data"][node_type] = {
                    "y:NodeLabel": [
                        f"{state.name}",
                        await self.getEvents(state)
                    ],
                    "y:Geometry": {
                        "@x": state.x,
                        "@y": state.y,
                        "@width": state.width,
                        "@height": state.height
                    }
                }

                xmlstate["graph"] = {}
                xmlstate["graph"]["node"] = []
                for child in state.childs:
                    xmlstate["graph"]["node"].append(await self._recursiveGetStates(child, xmlstate["graph"], parent=f"{state.id}::"))
            else:
                for child in state.childs:
                    graph["node"].append(await self._recursiveGetStates(child, graph))

        else:
            node_type = "y:GenericNode"
            xmlstate["data"][node_type] = {
                "y:NodeLabel": [
                    f"{state.name}",
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
            "node": [
                {
                    "@id": "",
                    "data": {
                        "y:GenericNode": {
                            "@configuration": "com.yworks.bpmn.Event.withShadow",
                            "y:NodeLabel": {}
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
            "graph": await self.getStates(list(states.values())),
        }
        }

        await self.addInitialState(initial_state)
        data["graphml"]["graph"]["edge"] = self.transitions
        result = xmltodict.unparse(data, pretty=True)

        return result
