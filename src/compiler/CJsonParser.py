from SourceFile import SourceFile
import asyncio
from fullgraphmlparser.stateclasses import State, Trigger
from component import Component
class CJsonParser:
    @staticmethod
    async def getLibraries(components):
        libraries = []
        for component in components:
            if component.type not in libraries:
                libraries.append(f"{component.type}")

        return libraries

    @staticmethod
    async def createNotes(components):
        includes = []
        variables = []
        for component in components:
            if component.type not in includes:
                includes.append(f'\n#include "{component.type}.h"')
            
            variables.append(f"\n{component.type} {component.name} = {component.type}({' '.join(map(str, list(component.parameters.values())))});")
        
        notes = []      
        notes.append({ "y:UMLNoteNode": 
                            {'y:NodeLabel' : 
                                {"#text" : f'Code for h-file: {"".join([*includes, *variables])}'}}}
                      )        
        
        return notes
    @staticmethod
    async def getComponents(components):
        result = []
        
        for component_name in components:
            result.append(Component(component_name, type=components[component_name]["type"], parameters=components[component_name]["parameters"]))
        
        return result
    
    @staticmethod
    async def getTransitions(transitions):
        result = {}
        i = 0
        for transition in transitions:
            name = f"trig{i}"
            guard = ''.join([transition["condition"]["component"],'.',transition["condition"]["method"],'('])
            if "args" in transition["condition"].keys():
                guard += ','.join(transition["condition"]["args"])
            guard += ')'
            result[name] = (Trigger(name=name, source=transition["source"], target=transition["target"], id=i, type="external", guard=guard, action="", points=[]))
            i += 1
        return result       
        
    
    @staticmethod
    async def getEvents(events):
        result = {}
        for eventname in events:
            command = ""
            for i in range(len(events[eventname])):
                command += events[eventname][i]["component"] + '.' + events[eventname][i]["method"] + '('
                if "args" in events[eventname][i].keys():
                    command += ','.join(events[eventname][i]["args"])
                command += ');'
            result[eventname] = command 

        return result
    
    @staticmethod
    def addTransitionsToStates(transitions, states):
        new_states = states.copy()
        for transition in transitions.values():
            print(transition)
            new_states[transition.source].trigs.append(transition)
        
        print(new_states)
        return new_states
    
    @staticmethod
    async def parseStateMachine(json_data):
        try:
            global_state = State(name="global", type="external", actions="", trigs=[], entry="", exit="", id="global", new_id=["global"], parent=None, childs=[])
            states = json_data["state"]
            proccesed_states = {}
            #Добавить parent?
            for statename in states:
                state = json_data["state"][statename]
                events = await CJsonParser.getEvents(state["events"])
                on_enter = ""
                on_exit = ""
                if "onExit" in events.keys():
                    on_exit = events["onExit"]
                if "onEnter" in events.keys():
                    on_enter = events["onEnter"]
                proccesed_states[statename] = State(name=statename, type="external", 
                                                    actions="", trigs=[], entry=on_enter, 
                                                    exit=on_exit, id=statename,
                                                    new_id=[statename], parent=global_state, childs = [])
                global_state.childs.append(proccesed_states[statename])
            transitions = await CJsonParser.getTransitions(json_data["transitions"])
            components = await CJsonParser.getComponents(json_data["components"])
            notes = await CJsonParser.createNotes(components)
            startNode = proccesed_states[json_data["initialState"]].id
            player_signals = list(transitions.keys())
            proccesed_states = CJsonParser.addTransitionsToStates(transitions, proccesed_states)
            return {"states" : [global_state, *list(proccesed_states.values())], 
                    "notes": notes, 
                    "startNode" : startNode, 
                    "playerSignals": player_signals}
            
            
        except KeyError:
            print("Invalid request")

    @staticmethod
    async def getFiles(json_data):
        files = []
        
        for data in json_data:
            files.append(SourceFile(data["filename"], data["extension"], data["fileContent"]))
        
        return files
    
    