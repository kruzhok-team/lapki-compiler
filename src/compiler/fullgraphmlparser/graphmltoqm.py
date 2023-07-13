import xmltodict
import qm
import graphml as gr
import service_files
from graphml_to_cpp import CppFileWriter
from logger import logging
import sys
import os
from typing import Union, List


def get_states_from_graphml(filename: str):
    """
    creates state list
    :return:
    """
    try:
        data = xmltodict.parse(open(filename).read())
    except FileNotFoundError:
        logging.error('File %s does not exist' % filename)
        return list(), 0, 0
    # get nodes from file
    flat_nodes = gr.get_flat_nodes(data)
    state_nodes = [node for node in flat_nodes if
                   gr.is_node_a_state(node) or gr.is_node_a_choice(node) or gr.is_node_a_group(node)]
    state_nodes.sort(key=lambda st: len(st['id']))
    gr.update_qroup_nodes(state_nodes)
    state_nodes.sort(key=gr.coord_sort)
    coords = gr.get_minmax_coord(state_nodes)  # get min and max coord and height and widt of scheme
    # create states from nodes and add internal triggers to list of signals and all functions to function list
    qm_states, player_signal = qm.create_states_from_nodes(state_nodes, coords, [], [])
    # get edges for external triggers
    flat_edges = gr.get_flat_edges(data)
    try:
        start, start_node, start_action = gr.get_start_node_data(flat_nodes, flat_edges)
    except ValueError:
        logging.error('UML-diagram %s.graphml does not have start node' % filename)
        return list(), 0, 0
    # add external trigger and update list of signals with them
    _ = qm.update_states_with_edges(qm_states, flat_edges, start, player_signal, coords[0], coords[1])
    return qm_states, coords[0], coords[1]

# test2

def main(filenames: Union[List[str], str]):

    player_signal = list()
    if not isinstance(filenames, list):
        filenames = [filenames]
    modelnames: List[str] = list()
    for filename in filenames:
        try:
            data = xmltodict.parse(open(filename).read())
            modelname = os.path.basename(filename)
            modelname = modelname.split('.')[0]
            modelname = modelname[0].lower() + modelname[1:]
            modelnames.append(modelname)
        except FileNotFoundError:
            logging.error('File %s does not exist' % filename)
            continue
        # get nodes from file
        flat_nodes = gr.get_flat_nodes(data)
        state_nodes = [node for node in flat_nodes if
                       gr.is_node_a_state(node) or gr.is_node_a_choice(node) or gr.is_node_a_group(node)]
        state_nodes.sort(key=lambda st: len(st['id']))
        gr.update_qroup_nodes(state_nodes)
        state_nodes.sort(key=gr.coord_sort)

        coords = gr.get_minmax_coord(state_nodes)      # get min and max coord and height and widt of scheme
        # create states from nodes and add internal triggers to list of signals and all functions to function list
        functions: List[str] = list()
        qm_states, player_signal = qm.create_states_from_nodes(state_nodes, coords, player_signal, functions)
        # get edges for external triggers
        flat_edges = gr.get_flat_edges(data)
        try:
            start, start_node, start_action = gr.get_start_node_data(flat_nodes, flat_edges)
        except ValueError:
            logging.error('UML-diagram %s.graphml does not have start node' % filename)
            continue
        # add external trigger and update list of signals with them
        player_signal = qm.update_states_with_edges(qm_states, flat_edges, start, player_signal, coords[0], coords[1])
        # get notes
        notes = [node for node in flat_nodes if gr.is_node_a_note(node)]
        # TODO(aeremin) Extract to separate file.
        CppFileWriter(modelname, start_node, start_action, qm_states, notes, player_signal).write_to_file(os.path.dirname(filename))

    service_files.create_files(os.path.dirname(filenames[0]), player_signal, modelname, functions)


if __name__ == '__main__':
    states = get_states_from_graphml(sys.argv[1:][0])
    main(sys.argv[1:])
