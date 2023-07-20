from fullgraphmlparser.graphml import get_start_node_data
from fullgraphmlparser.stateclasses import State, Trigger
from component import Component
import xmltodict
import asyncjson
import json
"""This class gets Berloga-graphml and returns States, Components, Transitions
    Returns:
        _type_: _description_
"""
class GraphmlParser:
    
    @staticmethod
    def getParentNode(group_node : dict) -> dict:
        return {
            '@id' : group_node["@id"],
            'data' : group_node["data"]
        }
    @staticmethod
    def getFlattenStates(xml : list[dict], states : list = []) -> dict:
        for node in xml:
            if "graph" in node.keys():
                states.append(GraphmlParser.getParentNode(node))
                GraphmlParser.getFlattenStates(node["graph"]["node"], states)
            else:
                states.append(node)
        
        return states
    
    @staticmethod
    def getEvents(state : dict, node_type : str) -> dict :
        events : str = state["data"][node_type]["y:NodeLabel"][1]
        events = events.split("\n")
        new_events = {}
        current_event = ""        
        for ev in events:

            if "/" in ev:
                new_events[ev[:-1]] = []
                current_event = ev[:-1].replace(' ', '')
                
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
            new_events[current_event].append(action_dict)    
        return new_events
    
    @staticmethod
    def getParentName(state : dict, flattenStates : list[dict]) -> str:
        id : str = state["@id"]
        pos = id.rfind(":")
        parent = ""
        if pos != -1:
            for s in flattenStates:
                if s["@id"] == id[:pos-1]:
                    try:
                        parent = s["data"]["y:GenericNode"]["y:NodeLabel"][0]
                    except KeyError:
                        parent = s["data"]["y:GroupNode"]["y:NodeLabel"][0]
        
        return parent
    
    @staticmethod
    def getEventFromTransition(action : str):
        events = action.split("/")
        result = {}
        for event in events:
    
    @staticmethod
    def getTriggers(triggers : dict, statesDict : dict) -> dict:
        transitions = []
        for trigger in triggers:
            transition = {}
            transition["source"] = statesDict[trigger["@source"]]    
            transition["target"] = statesDict[trigger["@target"]]
            transition["condition"] = {}
            transition["condition"]["position"] = {}
            component = GraphmlParser.getComponentsAndMethod(trigger["y:EdgeLabel"])
    @staticmethod
    def getGeometry(state : dict, type : str) -> dict:
        geometry = state["data"][type]["y:Geometry"]
        x = geometry['@x']
        y = geometry['@y']
        w = geometry['@width']
        h = geometry['@height']

        return {
            "x" : int(float(x)),
            "y" : int(float(y))
        }
        
    @staticmethod
    def createStates(flattenStates : list[dict]) -> list[State]:
        states = {}
        state_dictionary = {}
        for state in flattenStates:
            if 'y:GenericNode' in state["data"]:
                node_type = 'y:GenericNode'
            else:
                node_type = 'y:GroupNode'

            if state["@id"] == '':
                continue
            name = f"{state['data'][node_type]['y:NodeLabel'][0]}"
            new_state = {}
            state_dictionary[state["@id"]] = {}
            state_dictionary[state["@id"]]["name"] = name
            state_dictionary[state["@id"]]["type"] = node_type
            new_state["events"] = GraphmlParser.getEvents(state, node_type)
            new_state["bounds"] = GraphmlParser.getGeometry(state, node_type)
            parent = GraphmlParser.getParentName(state, flattenStates)
            if parent != "":
                new_state["parent"] = parent
            states[name] = new_state
        
        return states, state_dictionary

    @staticmethod
    def parse(unprocessed_xml : str):
        xml = xmltodict.parse(unprocessed_xml)
        graph = xml["graphml"]["graph"]
        nodes = graph["node"]
        triggers = graph["edge"]
        components = {}
        flattenStates = GraphmlParser.getFlattenStates(nodes)
        states, states_dict = GraphmlParser.createStates(flattenStates)
        triggers = GraphmlParser.getTriggers(triggers)
        # print(triggers)
        with open("biba.json", "w") as f:
            json.dump(states, f, ensure_ascii=False)
        return states, triggers, components