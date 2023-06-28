import sys
sys.path.append("/home/l140/programming/ide/compiler/src/compiler/")
sys.path.append("/home/l140/programming/ide/compiler/src/compiler/fullgraphmlparser")
print(sys.path)
from src.compiler.fullgraphmlparser.stateclasses import State, Trigger
from src.compiler.fullgraphmlparser.graphml_to_cpp import CppFileWriter

#Пример из MiRo

trig_from_off_to_on = Trigger(name="Trig1", source="Off", target="On", type="external", guard="btn.isJustPressed()", id=1, x=0, y=0, dx=0, dy=0, points=[], action="", action_x=0, action_y=0, action_width=0)
trig_from_on_to_off = Trigger(name="Trig2", source="On", target="Off", type="external", guard="btn.isJustPressed()", id=2, x=0, y=0, dx=0, dy=0, points=[], action="", action_x=0, action_y=0, action_width=0)
# trig_init = Trigger(name="init", source="initial", target="Off", type="external", guard="true", id=3, x=0, y=0, dx=0, dy=0, points=[], action="", action_x=0, action_y=0, action_width=0)
state1 = State(name="Off", type="internal", trigs=[trig_from_off_to_on], actions="", entry="diod.turnOff();", exit="", id="Off", new_id=["Off"], x=0, y=0, width=0, height=0, parent=None, childs=[])
state2 = State(name="On", type="internal", trigs=[trig_from_on_to_off], actions="", entry="diod.turnOn();", exit="", id="On", new_id=["On"], x=0, y=0, width=0, height=0, parent=None, childs=[])
state3 = State(name="init", type="external", trigs=[], actions="", entry="", exit="", id="init", new_id=["init"], x=0, y=0, width=0, height=0, parent=None, childs=[state1, state2])

# note = {"y:UMLNoteNode": {'y:NodeLabel' : {"#text" : 'Code for h-file: (do not delete this caption): \nsome pretty code here'}}}
note = {"y:UMLNoteNode": {'y:NodeLabel' : {"#text" : 'Code for h-file: \n#include "Led.h"'}}}
note1 = {"y:UMLNoteNode": {'y:NodeLabel' : {"#text" : 'Code for cpp-file: \nLed diod = Led(12);'}}}
CppFileWriter("test", start_node="init", start_action="", states=[state3, state2, state1], notes =[note, note1], player_signal=["Trig1", "Trig2"]).write_to_file("src")

# state = State(name="Bba", type="external", actions="", trigs=[], entry="", exit="", id="1", new_id=["1"], parent=None, childs=[])
