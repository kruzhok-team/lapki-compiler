
from dataclasses import dataclass
from typing import List, Tuple, Optional

"""
   Class Trigger describes Triggers of uml-diagrams
        name: name of trigger
        type: internal or external
        guard: text of trigger guard if any
        source: source state of trigger (actual for external triggers)
        target: target state of trigger (actual for external triggers)
        action: action for this trigger if any
        id: order number of internal trigger for better coordinates
        x, y: start of trigger visual path
        dx, dy: first relative movement of trigger visual path
        points: other relative movements of trigger visual path
        action_x, action_y, action_width: coordinates of trigger label
"""


@dataclass
class Trigger:
    name: str
    source: str
    target: str
    action: str
    id: int
    points: List[Tuple[int, int]]
    x: int = 0
    y: int = 0
    dx: int = 0
    dy: int = 0
    action_x: int = 0
    action_y: int = 0
    action_width: int = 0
    type: str = "internal"
    guard: str = ""


"""
   class State describes state of uml-diagram and trigslates to qm format.
   Fields:
        name: name of state
        type: state or choice
        trigs: list of trigsitions from this state both external and internal
        entry: action on entry event
        exit: action on exit event
        id: number of state
        actions: raw_data for external actions
        old_id: id of state in graphml
        x, y: graphical coordinates
        height, width: height and with of node
"""


@dataclass
class State:
    name: str
    type: str
    actions: str
    trigs: List[Trigger]
    entry: str
    exit: str
    id: str
    new_id: List[str]
    parent: Optional['State']
    childs: List['State']
    x: int = 0
    y: int = 0
    width: int = 0
    height: int = 0

    def __str__(self) -> str:
    
        if self.parent is not None:
            return f"{self.name, self.parent.name, ', '.join([child.name for child in self.childs]) }"
        else:
            return f"{self.name}, parent: None, {', '.join([child.name for child in self.childs])}"