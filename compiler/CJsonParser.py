import asyncio
from aiofile import async_open
from enum import Enum

try:
    from .SourceFile import SourceFile
    from .fullgraphmlparser.stateclasses import State, Trigger
    from .component import Component
    from .wrapper import to_async
except ImportError:
    from compiler.SourceFile import SourceFile
    from compiler.fullgraphmlparser.stateclasses import State, Trigger
    from compiler.component import Component
    from compiler.wrapper import to_async


class Labels(Enum):
    H = 'Code for h-file'
    CPP = 'Code for cpp-file'
    CTOR = 'Constructor code'

class CJsonParser:
    
    @staticmethod
    async def appendNote(label : Labels, content, notes):
        notes.append({ "y:UMLNoteNode": 
                    {'y:NodeLabel' : 
                        {"#text" : f'{label.value}: {content}'}}}
                )  
    
    
    @staticmethod
    async def getLibraries(components) -> list[str]:
        libraries = []
        for component in components:
            if component.type not in libraries:
                libraries.append(f"{component.type}")

        return libraries
    
    
    @staticmethod
    async def createNotes(components : list[Component], filename : str, triggers : list, compiler : str, path):
        includes = []
        variables = []
        setup = []
        for component in components:
            if component.type not in includes:
                includes.append(f'\n#include "{component.type}.h"')
              
            variables.append(f"\n{component.type} {component.name} = {component.type}({', '.join(map(str, list(component.parameters.values())))});")
        notes = []
        
        
        class_filename = filename[0].upper() + filename[1:]
        
        check_signals = []
        for name in triggers.keys():
            check_signals.append('\n\t\t'.join([f"\n\tif({triggers[name]})" "{", f"SIMPLE_DISPATCH(the_{filename}, {name});"]) + "\n\t}")
        
        match compiler:
            case "g++" | "gcc":
                setup_function = '\n\t'.join(["\nvoid setup(){",
                                            *setup,
                                            
                                            "\n}"])


                loop_function = ''.join(["\nvoid loop(){", *check_signals,
                                        "\n}"])

                main_function = '\n\t'.join(["\nint main(){", 
                                            f"{class_filename}_ctor();", 
                                            "QEvt event;",
                                            f"QMsm_init(the_{filename}, &event);",
                                            "setup();",
                                            "while(true){",
                                            "\tloop();",
                                            "}"]) + "\n}"
                
                await CJsonParser.appendNote(Labels.H, "".join([*includes, *variables]), notes)   
                await CJsonParser.appendNote(Labels.CPP, "\n\n".join([setup_function, loop_function, main_function]), notes)
            
            case "arduino-cli":
                setup_function = '\n\t'.join(["\nvoid setup(){",
                            f"{class_filename}_ctor();", 
                            "QEvt event;",
                            f"QMsm_init(the_{filename}, &event);",
                            "\n}"])
                loop_function = ''.join(["\nvoid loop(){", *check_signals,
                        "\n}"])
                await CJsonParser.appendNote(Labels.H, "".join([*includes, *variables]), notes)
                await CJsonParser.appendNote(Labels.CPP, "\n\n".join([setup_function, loop_function]), notes)

        return notes
    
    
    @staticmethod
    async def getComponents(components : list) -> list[Component]:
        result = []
        
        for component_name in components:
            result.append(Component(component_name, type=components[component_name]["type"], parameters=components[component_name]["parameters"]))
        
        return result

    
    
    @staticmethod
    async def getTransitions(transitions):
        result = []
        player_signals = {}
        i = 0
        for transition in transitions:
            name = ''.join([transition["condition"]["component"],'_',transition["condition"]["method"].upper()])
            guard = ''.join([transition["condition"]["component"],'.',transition["condition"]["method"],'('])
            if "args" in transition["condition"].keys():
                guard += ','.join(transition["condition"]["args"])
            guard += ')'
            
            trig = {}
            player_signals[name] = guard 
            trig["trigger"] = Trigger(name=name, source=transition["source"], target=transition["target"], id=i, type="external", guard="true", action="", points=[])
            
            result.append(trig)
            i += 1
        return result, player_signals
        
    
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
    def addParentsAndChilds(states, processed_states, global_state):
        result = processed_states.copy()
        for statename in states:
            state = states[statename]
            try:
                result[statename].parent = result[state["parent"]]
                result[state["parent"]].childs.append(result[statename])
            except KeyError:
                result[statename].parent = global_state
                global_state.childs.append(result[statename])
        
        return result
    
    
    @staticmethod
    def addTransitionsToStates(transitions, states):
        new_states = states.copy()
        for transition in transitions:
            new_states[transition["trigger"].source].trigs.append(transition["trigger"])
        
        return new_states
    
    
    @staticmethod
    async def parseStateMachine(json_data, filename, compiler, path=None):
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
                    
                    proccesed_states[statename] = State(name=statename, type="internal", 
                                                        actions="", trigs=[], entry=on_enter, 
                                                        exit=on_exit, id=statename,
                                                        new_id=[statename], parent=None, childs = [])
                transitions, player_signals = await CJsonParser.getTransitions(json_data["transitions"])
                components = await CJsonParser.getComponents(json_data["components"])
                notes = await CJsonParser.createNotes(components, filename, player_signals, compiler=compiler, path=path)
                startNode = proccesed_states[json_data["initialState"]].id
                
                proccesed_states = CJsonParser.addTransitionsToStates(transitions, proccesed_states)
                proccesed_states = CJsonParser.addParentsAndChilds(states, proccesed_states, global_state)
                
                return {"states" : [global_state, *list(proccesed_states.values())], 
                        "notes": notes, 
                        "startNode" : startNode, 
                        "playerSignals": player_signals.keys()}
                
                
            except KeyError as e:
                print(f"Invalid request, {e.args[0]} key doesn't support")
    
    @staticmethod
    async def getFiles(json_data):
        files = []
        
        for data in json_data:
            files.append(SourceFile(data["filename"], data["extension"], data["fileContent"]))
        
        return files
    
    
