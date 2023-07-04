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

state1 = State(name="Off", type="external", trigs=[trig_from_off_to_on], actions="", entry="diod.turnOff();", exit="", id="Off", new_id=["Off"], x=0, y=0, width=0, height=0, parent=None, childs=[])
state2 = State(name="On", type="external", trigs=[trig_from_on_to_off], actions="", entry="diod.turnOn();", exit="", id="On", new_id=["On"], x=0, y=0, width=0, height=0, parent=None, childs=[])
state3 = State(name="init", type="external", trigs=[], actions="", entry="", exit="", id="init", new_id=["init"], x=0, y=0, width=0, height=0, parent=None, childs=[state1, state2])


child_trig1 = Trigger(name="BUTTON_PRESSED", source="child1", target="child2", action="", id=0, points=[], type="internal")
child_trig2 = Trigger(name="BUTTON_PRESSED1", source="child2", target="Off", type="internal", id=1, action="", points=[])

child_state1 = State(name="child1", type="internal", id="child1", new_id=["child1"], childs=[], trigs=[child_trig1], parent=state2, actions="", entry="", exit="")

child_state2 = State(name="child2", type="internal", id="child2", new_id=["child2"], childs=[], trigs=[child_trig2], parent=state2, actions="", entry="", exit="")

state2.childs.append(child_state1)
state2.childs.append(child_state2)


state1.parent=state3
state2.parent=state3

# note = {"y:UMLNoteNode": {'y:NodeLabel' : {"#text" : 'Code for h-file: (do not delete this caption): \nsome pretty code here'}}}
note = {"y:UMLNoteNode": {'y:NodeLabel' : {"#text" : 'Code for h-file: \n#include "Led.h"'}}}
note1 = {"y:UMLNoteNode": {'y:NodeLabel' : {"#text" : 'Code for cpp-file: \nLed diod = Led(12);'}}}
CppFileWriter("test", start_node="init", start_action="", states=[state3, state2, state1, child_state1, child_state2], notes =[note, note1], player_signal=["BUTTON_PRESSED", "bibaBoba"]).write_to_file("test2")

# state = State(name="Bba", type="external", actions="", trigs=[], entry="", exit="", id="1", new_id=["1"], parent=None, childs=[])
