import xmltodict

try:
    from compiler.fullgraphmlparser.stateclasses import State, Trigger
except ImportError:
    from .fullgraphmlparser.stateclasses import State, Trigger



class JsonConverter:
    
    # TODO: Геометрия

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
        events.append("\n".join(["\nentry/", state.entry]))

        for trig in state.trigs:
            if trig.type == "internal":
                event = "\n".join([f"\n{trig.name}/", f"{trig.action}"])
                events.append(event)
            else:
                if trig.guard == "true":
                    transition = {
                                    "@source": trig.source,
                                    "@targer": trig.target,
                                    "y:EdgeLabel": f"{trig.name}/\n"
                                }
                else:
                    transition = {
                        "@source": trig.source,
                        "@targer": trig.target,
                        "y:EdgeLabel": f"{trig.name}/\n[{trig.guard}]"
                        }
                self.transitions.append(transition)
        events.append("\n".join(["\nexit/", state.exit]))
        return "".join(events)

    async def _recursiveGetStates(self, state: State, graph: dict) -> dict:
        xmlstate = {
            "@id": state.id,
            "data": {}
        }

        if len(state.childs) > 0:
            if state.name != "global":
                node_type = "y:GroupNode"
                xmlstate["data"][node_type] = {
                    "y:NodeLabel": [
                        state.name,
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
                    xmlstate["graph"]["node"].append(await self._recursiveGetStates(child, xmlstate["graph"]))
            else:
                for child in state.childs:
                    graph["node"].append(await self._recursiveGetStates(child, graph))

        else:
            node_type = "y:GenericNode"
            xmlstate["data"][node_type] = {
                "y:NodeLabel": [
                    state.name,
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
        
    async def parse(self, states: list[State], triggers: list[Trigger]) -> str:
        data = {"graphml": {
                    "@xmlns": "http://graphml.graphdrawing.org/xmlns",
                    "@xmlns:y": "http://www.yworks.com/xml/graphml",
                    "graph": await self.getStates(states),
                    "edge": self.transitions
                    }
                }
        with open("Berloga.xml", "w") as f:
            xmltodict.unparse(data, f, pretty=True)
        return data
