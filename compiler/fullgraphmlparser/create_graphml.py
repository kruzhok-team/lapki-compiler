from lxml import etree
import xmltodict
from typing import List, Tuple

Tag = etree._Element

namespace_dict = {None: "http://graphml.graphdrawing.org/xmlns",
                  'java': "http://www.yworks.com/xml/yfiles-common/1.0/java",
                  'sys': "http://www.yworks.com/xml/yfiles-common/markup/primitives/2.0",
                  'x': "http://www.yworks.com/xml/yfiles-common/markup/2.0",
                  'xsi': "http://www.w3.org/2001/XMLSchema-instance",
                  'y': "http://www.yworks.com/xml/graphml",
                  'yed': "http://www.yworks.com/xml/yed/3"}

scheme = "http://graphml.graphdrawing.org/xmlns http://www.yworks.com/xml/schema/graphml/1.1/ygraphml.xsd"
scheme_loc = "http://www.w3.org/2001/XMLSchema-instance"

prepare_dict = {'d0': {'attr.name': "Description", 'attr.type': "string", 'for': 'graph'},
                'd1': {'for': 'port', 'yfiles.type': 'portgraphics'},
                'd2': {'for': 'port', 'yfiles.type': 'portgeometry'},
                'd3': {'for': 'port', 'yfiles.type': 'portuserdata'},
                'd4': {'attr.name': "url", 'attr.type': "string", 'for': 'node'},
                'd5': {'attr.name': "description", 'attr.type': "string", 'for': 'node'},
                'd6': {'for': 'node', 'yfiles.type': 'nodegraphics'},
                'd7': {'for': 'graphml', 'yfiles.type': 'resources'},
                'd8': {'attr.name': "url", 'attr.type': "string", 'for': 'edge'},
                'd9': {'attr.name': "description", 'attr.type': "string", 'for': 'edge'},
                'd10': {'for': 'edge', 'yfiles.type': 'edgegraphics'},
                }

graph_dict = {'edgedefault': 'directed',
              'id': 'G'}

simple_node_color_dict = {'color': '#E8EEF7',
                          'color2': '#B7C9E3',
                          'transparent': "false"}

configuration = 'com.yworks.entityRelationship.big_entity'

node_border_dict = {'color': "#000000",
                    'type': "line",
                    'width': "1.0"}

node_label_dict = {'alignment': 'center',
                   'autoSizePolicy': "node_width",
                   'backgroundColor': "#B7C9E3",
                   'configuration': 'com.yworks.entityRelationship.label.name',
                   'fontFamily': 'Consolas',
                   'fontSize': '12',
                   'fontStyle': 'bold',
                   'hasLineColor': 'false',
                   'height': '18.7',
                   'horizontalTextPosition': 'center',
                   'iconTextGap': '4',
                   'modelName': 'internal',
                   'modelPosition': 't',
                   'textColor': "#000000",
                   "verticalTextPosition": 'bottom',
                   'visible': 'true',
                   'width': '200',
                   'x': '0.0',
                   'y': '4.0'}

node_content_dict = {'alignment': 'left',
                     'autoSizePolicy': 'content',
                     'hasBackgroundColor': "false",
                     'configuration': 'com.yworks.entityRelationship.label.attributes',
                     'fontFamily': 'Consolas',
                     'fontSize': '12',
                     'fontStyle': 'plane',
                     'hasLineColor': 'false',
                     'height': '40',
                     'horizontalTextPosition': 'center',
                     'iconTextGap': '4',
                     'modelName': 'custom',
                     # 'modelPosition':'tl',
                     'textColor': "#000000",
                     "verticalTextPosition": 'bottom',
                     'visible': 'true',
                     'width': '43.5',
                     'x': '0.0',
                     'y': '4.0'}

group_node_label_dict = {'alignment': "center",
                         'autoSizePolicy': "node_width",
                         'backgroundColor': "#EBEBEB",
                         'borderDistance': "0.0",
                         'fontFamily': "Consolas",
                         'fontSize': "15",
                         'fontStyle': "plain",
                         'hasLineColor': "false",
                         'height': "22.5",
                         'horizontalTextPosition': "center",
                         'iconTextGap': "4",
                         'modelName': "internal",
                         'modelPosition': "t",
                         'textColor': "#000000",
                         'verticalTextPosition': "bottom",
                         'visible': "true",
                         'width': "252.7",
                         'x': "0.0",
                         'y': "0.0"}

group_node_dict = {'alignment': "left",
                   'autoSizePolicy': "node_size",
                   'fontFamily': "Consolas",
                   'fontSize': "12",
                   'fontStyle': "plain",
                   'hasBackgroundColor': "false",
                   'hasLineColor': "false",
                   'height': "223.75",
                   'horizontalTextPosition': "center",
                   'iconTextGap': "4",
                   'modelName': "custom",
                   # 'modelPosition': "t",
                   'textColor': "#000000",
                   'verticalTextPosition': "bottom",
                   'visible': "true",
                   'width': "252.68",
                   'x': "0.0",
                   'y': "4.0"}

style1_dict = {'class': "java.lang.Boolean",
               "name": "y.view.ShadowNodePainter.SHADOW_PAINTING",
               "value": "true"}

style2_dict = {'class': "java.lang.Boolean",
               "name": "doubleBorder",
               "value": "false"}

group_node_color_dict = {'color': '#F5F5F5',
                         'transparent': "false"}

group_node_state_dict = {'closed': "false",
                         'closedHeight': "50.0",
                         'closedWidth': "50.0",
                         'innerGraphDisplayEnabled': "false"}

group_node_inset_dict = {'bottom': "15",
                         'bottomF': "15.0",
                         'left': "15",
                         'leftF': "15.0",
                         'right': "15",
                         'rightF': "15.0",
                         'top': "15",
                         'topF': "15.0"}

group_node_borders_dict = {'bottom': "47",
                           'bottomF': "46.9",
                           'left': "11",
                           'leftF': "11.35",
                           'right': "11",
                           'rightF': "11.35",
                           'top': "25",
                           'topF': "24.5"}

linestyle_dict = {'color': '#000000',
                  'type': 'line',
                  'width': '1.0'}

arrows_dict = {'source': 'none',
               'target': 'standart'}

edge_dict = {'alignment': "left",
             'backgroundColor': "#FFFFFF",
             'configuration': "AutoFlippingLabel",
             'distance': "2.0",
             'fontFamily': "Consolas",
             'fontSize': "12",
             'fontStyle': "plain",
             'hasLineColor': "false",
             'height': "18.7",
             'horizontalTextPosition': "center",
             'iconTextGap': "4",
             'modelName': "free",
             'modelPosition': "anywhere",
             'preferredPlacement': "anywhere",
             'ratio': "0.5",
             'textColor': "#000000",
             'verticalTextPosition': "bottom",
             'visible': "true",
             'width': "58.66",
             'x': "45.64",
             'y': "-9.35"}

placement_dict = {'angle': "0.0",
                  'angleOffsetOnRightSide': "0",
                  'angleReference': "absolute",
                  'angleRotationOnRightSide': "co",
                  'distance': "-1.0",
                  'frozen': "true",
                  'placement': "anywhere",
                  'side': "anywhere",
                  'sideReference': "relative_to_edge_flow"}


def prepare_graphml() -> Tag:
    """
    prepares graphml tag with parameters and all tags before graph tag
    :return: root element
    """
    attr_qname = etree.QName(scheme_loc, "schemaLocation")
    graphml_root = etree.Element("graphml",
                                 {attr_qname: scheme},
                                 nsmap=namespace_dict)
    for key in prepare_dict.keys():
        _ = etree.SubElement(graphml_root, 'key', id=key, **prepare_dict[key])
    return graphml_root


def create_graph(tree_root: Tag, param: str) -> Tag:
    """
    adds graph tag to root
    :param tree_root: root element of graphml
    :param param: graphparameter,
    :return: graph element
    """
    graph_dict['id'] = param
    graph_node = etree.SubElement(tree_root, 'graph', **graph_dict)
    _ = etree.SubElement(graph_node, 'data', key='d0')
    return graph_node


def add_simple_node(parent: Tag, node_text: str, content: str, node_id: str,
                    h: int, w: int, x0: float, y0: float):
    """
    creates simple node with parameters
    :param parent:
    :param node_text: node labes
    :param content: node content
    :param node_id: node id
    :param h: height
    :param w: width
    :param x0: x of left upper cornet
    :param y0: y of lest upper cornet
    :return:
    """
    node = etree.SubElement(parent, "node", id=node_id)

    data = etree.SubElement(node, "data", key="d4")
    data.set("{http://www.w3.org/XML/1998/namespace}space", "preserve")

    data_full = etree.SubElement(node, "data", key="d6")
    nmspc_y = namespace_dict['y']

    generic_node = etree.SubElement(data_full, etree.QName(nmspc_y, "GenericNode"), configuration=configuration)
    _ = etree.SubElement(generic_node, etree.QName(nmspc_y, "Geometry"),
                         height=str(h), width=str(w), x=str(x0), y=str(y0))
    _ = etree.SubElement(generic_node, etree.QName(nmspc_y, 'Fill'), **simple_node_color_dict)
    _ = etree.SubElement(generic_node, etree.QName(nmspc_y, 'BorderStyle'), **node_border_dict)

    nodelabel = etree.SubElement(generic_node, etree.QName(nmspc_y, 'NodeLabel'), **node_label_dict)
    nodelabel.set("{http://www.w3.org/XML/1998/namespace}space", "preserve")
    nodelabel.text = node_text

    nodecontent = etree.SubElement(generic_node, etree.QName(nmspc_y, 'NodeLabel'), **node_content_dict)
    nodecontent.set("{http://www.w3.org/XML/1998/namespace}space", "preserve")
    nodecontent.text = content

    label_model = etree.SubElement(nodecontent, etree.QName(nmspc_y, "LabelModel"))
    _ = etree.SubElement(label_model, etree.QName(nmspc_y, "ErdAttributesNodeLabelModel"))
    model_param = etree.SubElement(nodecontent, etree.QName(nmspc_y, "ModelParameter"))
    _ = etree.SubElement(model_param, etree.QName(nmspc_y, "ErdAttributesNodeLabelModelParameter"))

    nodestyle = etree.SubElement(generic_node, etree.QName(nmspc_y, 'StyleProperties'))
    _ = etree.SubElement(nodestyle, etree.QName(nmspc_y, 'Property'), **style1_dict)
    _ = etree.SubElement(nodestyle, etree.QName(nmspc_y, 'Property'), **style2_dict)


def add_group_node(parent: Tag, node_text: str, content: str, node_id: str,
                   h: int, w: int, x0: float, y0: float) -> Tag:
    """
    creates group node with parameters
    :param parent:
    :param node_text: node labes
    :param content: node content
    :param node_id: node id
    :param h: height
    :param w: width
    :param x0: x of left upper cornet
    :param y0: y of lest upper cornet
    :return: group node tag
    """
    node_dict = {'id': node_id, "yfiles.foldertype": "group"}
    nmspc_y = namespace_dict['y']

    node = etree.SubElement(parent, "node", **node_dict)
    data = etree.SubElement(node, 'data', key="d6")
    proxy = etree.SubElement(data, etree.QName(nmspc_y, "ProxyAutoBoundsNode"))
    realizers = etree.SubElement(proxy, etree.QName(nmspc_y, "Realizers"), active='0')

    group_node = etree.SubElement(realizers, etree.QName(nmspc_y, "GroupNode"))
    _ = etree.SubElement(group_node, etree.QName(nmspc_y, "Geometry"),
                         height=str(h), width=str(w), x=str(x0), y=str(y0))
    _ = etree.SubElement(group_node, etree.QName(nmspc_y, 'Fill'), **group_node_color_dict)
    _ = etree.SubElement(group_node, etree.QName(nmspc_y, 'BorderStyle'), **node_border_dict)

    nodelabel = etree.SubElement(group_node, etree.QName(nmspc_y, "NodeLabel"), **group_node_label_dict)
    nodelabel.set("{http://www.w3.org/XML/1998/namespace}space", "preserve")
    nodelabel.text = node_text

    nodecontent = etree.SubElement(group_node, etree.QName(nmspc_y, 'NodeLabel'), **group_node_dict)
    nodecontent.set("{http://www.w3.org/XML/1998/namespace}space", "preserve")
    nodecontent.text = content

    label_model = etree.SubElement(nodecontent, etree.QName(nmspc_y, "LabelModel"))
    _ = etree.SubElement(label_model, etree.QName(nmspc_y, "ErdAttributesNodeLabelModel"))
    model_param = etree.SubElement(nodecontent, etree.QName(nmspc_y, "ModelParameter"))
    _ = etree.SubElement(model_param, etree.QName(nmspc_y, "ErdAttributesNodeLabelModelParameter"))

    _ = etree.SubElement(group_node, etree.QName(nmspc_y, "Shape"), type="roundrectangle")
    _ = etree.SubElement(group_node, etree.QName(nmspc_y, "State"), **group_node_state_dict)
    _ = etree.SubElement(group_node, etree.QName(nmspc_y, "NodeBounds"), considerNodeLabelSize='true')
    _ = etree.SubElement(group_node, etree.QName(nmspc_y, "Insets"), **group_node_inset_dict)
    _ = etree.SubElement(group_node, etree.QName(nmspc_y, "BorderInsets"), **group_node_borders_dict)
    return node


def add_edge(parent: Tag, edge_id: str, source: str, target: str, text: str,
             x1: float, y1: float, x2: float, y2: float, points: List[Tuple[int, int]]):
    """
    adds edge to xml
    :param points: list of trigger points
    :param parent: parent tag
    :param edge_id: edge id
    :param source: source node id
    :param target: target node id
    :param text: edge label
    :param x1: start edge x
    :param y1: start edge y
    :param x2: finish edge x
    :param y2: finish edge y
    :return:
    """
    edge = etree.SubElement(parent, "edge", id=edge_id, source=source, target=target)
    _ = etree.SubElement(edge, "data", key="d9")
    data = etree.SubElement(edge, "data", key="d10")
    nmspc_y = namespace_dict['y']
    polyline_edge = etree.SubElement(data, etree.QName(nmspc_y, 'PolyLineEdge'))
    path = etree.SubElement(polyline_edge, etree.QName(nmspc_y, 'Path'), sx=str(x1), sy=str(y1), tx=str(x2), ty=str(y2))
    if points:
        for point in points:
            _ = etree.SubElement(path, etree.QName(nmspc_y, 'Point'), x=point[0], y=point[1])
    _ = etree.SubElement(polyline_edge, etree.QName(nmspc_y, 'LineStyle'), **linestyle_dict)
    _ = etree.SubElement(polyline_edge, etree.QName(nmspc_y, 'Arrows'), **arrows_dict)
    edgelabel = etree.SubElement(polyline_edge, etree.QName(nmspc_y, 'EdgeLabel'), **edge_dict)
    edgelabel.set("{http://www.w3.org/XML/1998/namespace}space", "preserve")
    edgelabel.text = text
    _ = etree.SubElement(edgelabel, etree.QName(nmspc_y, 'PreferredPlacementDescriptor'), **placement_dict)
    _ = etree.SubElement(polyline_edge, etree.QName(nmspc_y, "BendStyle"), smoothed="false")


def add_start_state(parent: Tag, node_id: str):
    """
    adds start state from template
    :param node_id: node_id
    :param parent: parent tag
    :return:
    """
    data = xmltodict.parse(open(r'graphml_templates/start_template.xml').read())
    node = etree.SubElement(parent, "node", id=node_id)
    data_tag = etree.SubElement(node, "data", key="d6")
    get_tags_from_template(data_tag, data, "")


def add_choice_state(parent: Tag, node_id: str):
    """
    adds start state from template
    :param node_id: node_id
    :param parent: parent tag
    :return:
    """
    data = xmltodict.parse(open(r'graphml_templates/choice_template.xml').read())
    node = etree.SubElement(parent, "node", id=node_id)
    data_tag = etree.SubElement(node, "data", key="d6")
    get_tags_from_template(data_tag, data, "")


def add_comment_state_with_text(parent: Tag, node_id: str, text: str):
    """
    adds comment node from template with added text to parent Node
    :param parent: parent node
    :param node_id: node id
    :param text: text to add
    :return:
    """
    data = xmltodict.parse(open(r'graphml_templates/comment_template.xml').read())
    node = etree.SubElement(parent, "node", id=node_id)
    data_tag = etree.SubElement(node, "data", key="d6")
    node.set("{http://www.w3.org/XML/1998/namespace}space", "preserve")
    get_tags_from_template(data_tag, data, text)


def get_state_fields_comment(parent: Tag, node_id: str, text: str):
    """
    gets comment for state fields
    :param node_id: od of node
    :param parent: tag to add comment
    :param text: text to add to states
    :return:
    """
    add_comment_state_with_text(parent, node_id, "State fields: (do not delete this caption):\n" + text)


def get_constructor_code_comment(parent: Tag, node_id: str,  text: str):
    """
    gets comment for constructor code
    :param parent: tag to add comment
    :param node_id: od of node
    :param text: text to add to constructor code
    :return:
    """
    add_comment_state_with_text(parent, node_id, "Constructor code: (do not delete this caption):\n" + text)


def get_h_code_comment(parent: Tag, node_id: str,  text: str):
    """
    gets comment for code for h-file
    :param parent: tag to add comment
    :param node_id: od of node
    :param text: text to add to h-file
    :return:
    """
    add_comment_state_with_text(parent, node_id, "Code for h-file: (do not delete this caption):\n" + text)


def get_c_code_comment(parent: Tag, node_id: str,  text: str):
    """
    gets comment for code for cpp-file
    :param parent: tag to add comment
    :param node_id: od of node
    :param text: text to add to cpp-file
    :return:
    """
    add_comment_state_with_text(parent, node_id, "Code for cpp-file: (do not delete this caption):\n" + text)


def get_constructor_fields_comment(parent: Tag, node_id: str,  text: str):
    """
    gets comment for additional  constructor fields
    :param parent: tag to add comment
    :param node_id: od of node
    :param text: list of constructor fields
    :return:
    """
    add_comment_state_with_text(parent, node_id, "Constructor fields: (do not delete this caption):\n" + text)


def get_event_fields_comment(parent: Tag, node_id: str,  text: str):
    """
    gets comment for additional event fields
    :param parent: tag to add comment
    :param node_id: od of node
    :param text: list of event fields
    :return:
    """
    add_comment_state_with_text(parent, node_id, "Event fields: (do not delete this caption):\n" + text)


def get_tags_from_template(parent: Tag, data: dict, text: str):
    """
    gets tag structure from template and adds it to tag structure, adding text to nodelabel
    :param parent: parent tag
    :param data: data with tags template
    :param text: text to add to node label
    :return:
    """
    for tag in data.keys():
        if isinstance(data[tag], dict):
            attributes = {key.replace("@", ""): data[tag][key] for key in data[tag].keys() if
                          not isinstance(data[tag][key], dict) and not isinstance(data[tag][key], list)}
            new_tag = etree.SubElement(parent, etree.QName(namespace_dict['y'], tag.replace("y:", "")), **attributes)
            if tag == 'y:NodeLabel':
                new_tag.text = text

            get_tags_from_template(new_tag, data[tag], text)
        if isinstance(data[tag], list):
            for i in range(len(data[tag])):
                if isinstance(data[tag][i], dict):
                    get_tags_from_template(parent, {tag: data[tag][i]}, text)


def finish_graphml(root: Tag):
    """
    creates finish tag
    :param root: root node
    :return:
    """
    data = etree.SubElement(root, 'data', key='d7')
    _ = etree.SubElement(data, etree.QName(namespace_dict['y'], 'Resources'))


if __name__ == '__main__':
    root_node = prepare_graphml()
    graph = create_graph(root_node, 'G')
    add_start_state(graph, "n2")
    group_node = add_group_node(graph, "parent", "parent_text", 'n0', 223, 252, 347, 152)
    group_graph = create_graph(group_node, 'n0:')
    add_simple_node(group_graph, 'idle', 'lorem ipsum', 'n0::n0', 100, 200, 374, 214)
    add_simple_node(graph, 'not_idle', 'lorem ipsum', 'n1', 100, 200, 734, 213)
    add_choice_state(graph, "n3")
    add_edge(graph, "e0", "n0::n0", "n1", 'TEST TRIGGER', 0, 0, 0, 0)
    add_edge(graph, "e1", "n2", "n1", "initial", 0, 0, 0, 0)
    add_edge(graph, "e2", "n1", "n3", "choice_trigger", 0, 0, 0, 0)
    add_edge(graph, "e3", "n3", "n0::n0", "guard1", 0, 0, 0, 0)
    add_edge(graph, "e4", "n3", "n0", "guard2", 0, 0, 0, 0)
    get_c_code_comment(graph, "n4", "some code here")
    get_constructor_code_comment(graph, 'n5', "me->test_field = test")
    get_constructor_fields_comment(graph, 'n6', "unsigned int: test_constructor")
    get_event_fields_comment(graph, 'n7', "unsigned int: test_event_field")
    get_h_code_comment(graph, "n8", "some code for h file here")
    get_state_fields_comment(graph, "n9", "unsigned int test_state_field")
    finish_graphml(root_node)
    xml_tree = etree.ElementTree(root_node)
    xml_tree.write("test.graphml", xml_declaration=True, pretty_print=True, encoding="UTF-8")
