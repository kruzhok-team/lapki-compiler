# from collections import OrderedDict
# import os
# from pathlib import Path
# import sys
# import unittest

# import xmltodict

# import cpp_to_graphml


# class StateMachineParserTest(unittest.TestCase):

#     def testNumberOfStates(self):
#         parser = cpp_to_graphml.StateMachineParser(cpp_file_path = './testdata/oregonPlayer.cpp')
#         sm = parser.Parse()
#         self.assertEqual(len(sm.states), 15)

#     def testHeaderParsed(self):
#         parser = cpp_to_graphml.StateMachineParser(cpp_file_path = './testdata/oregonPlayer.cpp')
#         sm = parser.Parse()
#         self.assertTrue(sm.raw_h_code)
#         self.assertTrue(sm.raw_h_code.startswith('#define'))
#         self.assertTrue(sm.raw_h_code.endswith('(DEFAULT_HP/3)\n'))

#         self.assertEqual(sm.event_fields, 'unsigned int value;')
#         self.assertEqual(sm.state_fields, '''unsigned int CharHP;
# QStateHandler StartState;
# unsigned int TimerAgony;''')
#         self.assertEqual(sm.constructor_fields, '''unsigned int HP;
# unsigned int State;
# unsigned int TimerAgony;''')

#         self.assertTrue(sm.constructor_code)
#         self.assertTrue(sm.constructor_code.startswith('me->CharHP = HP;'), sm.constructor_code)
#         self.assertTrue(sm.constructor_code.endswith('}'))

#         self.assertEqual(sm.initial_code, 'return Q_TRAN(me->StartState);')
#         self.assertEqual(sm.initial_state, 'healthy')

# class StateMachineWriterTest(unittest.TestCase):
#     OUTPUT_FILE = './testdata/output.graphml'
#     def removeOutputFile(self):
#         if (os.path.exists(self.OUTPUT_FILE)):
#             os.remove(self.OUTPUT_FILE)

#     def setUp(self):
#         self.removeOutputFile()

#     def tearDown(self):
#         self.removeOutputFile()

#     def testValidGraphml(self):
#         parser = cpp_to_graphml.StateMachineParser(cpp_file_path = './testdata/oregonPlayer.cpp')
#         cpp_to_graphml.StateMachineWriter(parser.Parse()).WriteToFile(self.OUTPUT_FILE)
#         output_file = Path(self.OUTPUT_FILE)
#         # Test that output file is present ...
#         self.assertTrue(output_file.is_file())

#         # ... but in addition test that it's a valid XML containing something graphml-looking.
#         with open(self.OUTPUT_FILE, 'r') as f:
#             xml_dict = xmltodict.parse(f.read())
#             self.assertTrue(xml_dict)
#             self.assertIsInstance(xml_dict['graphml'], OrderedDict)

# if __name__ == '__main__':
#     unittest.main()
