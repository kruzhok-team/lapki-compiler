import sys
sys.path.append("./src/compiler/")
from src.compiler.GraphmlParser import GraphmlParser


with open("Autoborder_638213644305392731.graphml", "r") as f:
    data = f.read()
    
GraphmlParser.parse(data)