"""Module for work with CyberiadaMl."""
from cyberiadaml_py.cyberiadaml_parser import CGMLParser
from fullgraphmlparser.stateclasses import StateMachine


def parse(xml: str) -> StateMachine:
    parser = CGMLParser()
    parser.parseCGML(xml)