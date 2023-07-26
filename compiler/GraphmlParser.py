import xmltodict
import json
from aiofile import async_open

try:
    from .config import SCHEMA_DIRECTORY
except ImportError:
    from compiler.config import SCHEMA_DIRECTORY

"""
    This class gets Berloga-graphml and returns States, Components, Transitions
    Returns:
        _type_: _description_
"""
class GraphmlParser:

    operatorAlias = {
        "!=": "notEquals",
        "==": "equals",
        ">": "greater",
        "<": "less",
        ">=": "greaterOrEqual",
        "<=": "lessOrEqual"
    }

    @staticmethod
    async def getParentNode(group_node: dict) -> dict:
        return {
            '@id': group_node["@id"],
            'data': group_node["data"]
        }

    @staticmethod
    async def addStateToDict(state: dict, states_dict: dict) -> None:
        if 'y:GenericNode' in state["data"]:
            node_type = 'y:GenericNode'
        else:
            node_type = 'y:GroupNode'

        states_dict[state["@id"]] = {}
        states_dict[state["@id"]]["type"] = node_type
        try:
            states_dict[state["@id"]]["name"] = state["data"][node_type]["y:NodeLabel"][0]
        except TypeError:
            pass

    @staticmethod
    async def getFlattenStates(xml: list[dict], states: list = [], states_dict: dict[str, dict[str, str]] = {}) -> tuple[list[dict[str, str | dict]], dict[str, dict[str, str]]]:
        for node in xml:
            if "graph" in node.keys():
                parent = await GraphmlParser.getParentNode(node)
                states.append(parent)
                await GraphmlParser.addStateToDict(parent, states_dict)
                await GraphmlParser.getFlattenStates(node["graph"]["node"], states, states_dict)
            else:
                await GraphmlParser.addStateToDict(node, states_dict)
                states.append(node)

        return states, states_dict

    @staticmethod
    async def getEvents(state: dict, node_type: str) -> dict[str, list[dict[str, str]]]:
        str_events: str = state["data"][node_type]["y:NodeLabel"][1]
        events: list[str] = str_events.split("\n")
        new_events: dict[str, list[dict[str, str | list]]] = {}
        current_event: str = ""
        new_events["componentSignals"] = []
        i = -1
        for ev in events:
            if "/" in ev:
                if "." in ev:
                    current_event = "componentSignals"
                    i += 1
                    command = ev.split(".")
                    component = command[0]
                    method = command[1][:-1]
                    new_events[current_event].append({
                        "component": component,
                        "method": method,
                        "actions": []
                    })
                    current_dict = new_events[current_event][i]["actions"]
                else:
                    current_event = ev[:-1].replace(' ', '')
                    if current_event == "entry":
                        current_event = "onEnter"
                    elif current_event == "exit":
                        current_event = "onExit"
                    new_events[current_event] = []
                    current_dict = new_events[current_event]
                continue

            action_dict = {}
            action = ev.split(".")
            component = action[0]
            action_dict["component"] = component
            bracket_pos = action[1].find("(")
            method = action[1][:bracket_pos]
            action_dict["method"] = method
            if bracket_pos != -1:
                args = action[1][bracket_pos+1:-1].split(",")
            if args != ['']:
                action_dict["args"] = args
            else:
                action_dict["args"] = []

            current_dict.append(action_dict)
        return new_events

    @staticmethod
    async def getParentName(state: dict, states_dict: dict) -> str:
        id: str = state["@id"]
        pos = id.rfind(":")
        parent = ""
        if pos != -1:
            parent = id[:pos-1]

        return parent

    @staticmethod
    async def checkValueType(value: str) -> dict:
        if "." in value:
            command = value.split(".")
            component = command[0]
            method = command[1]
            return {"type": "component",
                    "component": component,
                    "method": method,
                    "args": []}
        else:
            return {"type": "value",
                    "value": value}
        
    @staticmethod
    async def getCondition(condition: str) -> str:
        result = {}
        if condition != "":
            condition = condition.replace("[", "").replace("]", "")
            condition = condition.split()
            lval = await GraphmlParser.checkValueType(condition[0])
            operator = condition[1]
            rval = await GraphmlParser.checkValueType(condition[2])

            result = {
                "type": GraphmlParser.operatorAlias[operator],
                "values": [lval, rval]
            }

        return result

    @staticmethod
    async def calculateEdgePosition(source_position: dict, target_position: dict, used_coordinates: list) -> dict[str, int]:
        x1, y1, w1, h1 = list(source_position.values())
        x2, y2, w2, h2 = list(target_position.values())

        nx = x1 + w1 // 2
        ny = (y1 + y2 + h2) // 2

        while (nx, ny) in used_coordinates:
            ny += 100
            nx += 100
        used_coordinates.append((nx, ny))
        return {"x": nx, "y": ny}

    @staticmethod
    async def getTransitions(triggers: dict, statesDict: dict) -> tuple[list, str]:
        transitions = []
        initial_state = ""
        used_coordinates: list[tuple[int, int]] = []
        print(triggers)
        for trigger in triggers:
            transition = {}
            try:
                transition["source"] = trigger["@source"]
                transition["target"] = trigger["@target"]

                event, condition = trigger["y:EdgeLabel"].split("/")
                component, method = event.split(".")
                transition["trigger"] = {
                    "component": component,
                    "method": method
                }
                
                transition["condition"] = await GraphmlParser.getCondition(condition)
                source_geometry = statesDict[trigger["@source"]]["geometry"]
                target_geometry = statesDict[trigger["@target"]]["geometry"]
                transition["position"] = await GraphmlParser.calculateEdgePosition(source_geometry, target_geometry, used_coordinates)
                transition["do"] = []
                transitions.append(transition)
            except AttributeError:
                initial_state = trigger["@target"]

        return transitions, initial_state

    @staticmethod
    async def getGeometry(state: dict, type: str) -> dict:
        geometry = state["data"][type]["y:Geometry"]
        x = geometry['@x']
        y = geometry['@y']
        w = geometry['@width']
        h = geometry['@height']

        return {
            "x": int(float(x)),
            "y": int(float(y)),
            "w": int(float(w)),
            "h": int(float(h))
        }

    @staticmethod
    async def createStates(flattenStates: list[dict], states_dict: dict) -> dict:
        states = {}
        for state in flattenStates:
            if state["@id"] == '':
                continue
            new_state = {}
            id = state["@id"]
            node_type = states_dict[state["@id"]]["type"]
            new_state["name"] = states_dict[state["@id"]]["name"]
            new_state["events"] = await GraphmlParser.getEvents(state, node_type)
            geometry = await GraphmlParser.getGeometry(state, node_type)
            states_dict[state["@id"]]["geometry"] = geometry
            new_state["bounds"] = {"x": geometry["x"], "y": geometry["y"]}
            parent = await GraphmlParser.getParentName(state, states_dict)
            if parent != "":
                new_state["parent"] = parent
            states[id] = new_state

        return states

    @staticmethod
    async def getComponents() -> dict:
        async with async_open(f"{SCHEMA_DIRECTORY}Berloga.json", "r") as f:
            data = await f.read()
            json_data: dict = json.loads(data)

        result = {}
        components = json_data["platform"]["Берлога/Защита пасеки"]["components"].keys()

        for component in components:
            result[component] = {}
            result[component]["type"] = component
            result[component]["parameters"] = {}

        return result

    @staticmethod
    async def parse(unprocessed_xml: str):
        xml = xmltodict.parse(unprocessed_xml)
        graph = xml["graphml"]["graph"]
        nodes = graph["node"]
        triggers = graph["edge"]
        components = await GraphmlParser.getComponents()
        flattenStates, states_dict = await GraphmlParser.getFlattenStates(nodes)
        states = await GraphmlParser.createStates(flattenStates, states_dict)
        transitions, initial_state = await GraphmlParser.getTransitions(triggers, states_dict)

        return {"states": states,
                "initialState": initial_state,
                "transitions": transitions,
                "components": components}
