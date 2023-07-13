# import os
# import shutil
# import unittest

# import cpp_to_graphml
# import graphmltoqm
# import test_utils

# # Does full conversion cycle (C++ --> graphml --> qm --> C++)
# class CircularConsistencyTest(unittest.TestCase):
#     def setUp(self):
#         test_utils.removeOutputFolder()
#         os.makedirs('./testdata/test_output')

#     def tearDown(self):
#         test_utils.removeOutputFolder()

#     def testFullCycle(self):
#         parser = cpp_to_graphml.StateMachineParser(cpp_file_path='./testdata/oregonPlayer.cpp')
#         sm1 = parser.Parse()
#         cpp_to_graphml.StateMachineWriter(sm1).WriteToFile('./testdata/test_output/oregonPlayer.graphml',
#                                                            './testdata/oregonPlayer.graphml')
#         graphmltoqm.main('./testdata/test_output/oregonPlayer.graphml')
#         shutil.copy('./testdata/qhsm.h', './testdata/test_output')
#         shutil.copy('./testdata/eventHandlers.h', './testdata/test_output')
#         parser2 = cpp_to_graphml.StateMachineParser(cpp_file_path='./testdata/test_output/oregonPlayer.cpp')
#         sm2 = parser2.Parse()

#         self.assertEqual(len(sm1.states), len(sm2.states))
#         self.assertEqual(sm1.state_fields, sm2.state_fields)
#         self.assertEqual(sm1.event_fields, sm2.event_fields)
#         self.assertEqual(sm1.constructor_fields, sm2.constructor_fields)
#         self.assertEqual(sm1.constructor_code, sm2.constructor_code)
#         self.assertEqual(sm1.raw_h_code, sm2.raw_h_code)
#         self.assertEqual(sm1.initial_state, sm2.initial_state)
#         self.assertEqual(sm1.initial_code, sm2.initial_code)
#         for state_name in sorted(sm1.states.keys()):
#             s1 = sm1.states[state_name]
#             self.assertIn(state_name, sm2.states)
#             s2 = sm2.states[state_name]
#             self.assertEqual(s1.state_name, s2.state_name)
#             self.assertEqual(s1.parent_state_name, s2.parent_state_name, s1.state_name)

#             def names(state_list):
#                 return [s.state_name for s in state_list]

#             self.assertEqual(names(s1.child_states), names(s2.child_states))
#             self.assertCountEqual(s1.event_handlers, s2.event_handlers)

#         self.maxDiff = None

#         # This one is quite fragile as literally any change in the output will break it. Not sure if it's actually needed.
#         with open('./testdata/oregonPlayer.cpp', 'r') as f:
#             sm1_cpp_content = f.read()
#         with open('./testdata/test_output/oregonPlayer.cpp', 'r') as f:
#             sm2_cpp_content = f.read()
#         self.assertEqual(sm1_cpp_content, sm2_cpp_content)

# if __name__ == '__main__':
#     unittest.main()
