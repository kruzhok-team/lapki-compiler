import unittest

from qm import *


class TriggersCreateTest(unittest.TestCase):

    def testEmptyTriggers(self):
        res, _ = create_actions(" ", "test", [], [])
        self.assertEqual(len(res), 0)

    def testEntryOnly(self):
        res, _ = create_actions("entry/", "test", [], [])
        self.assertEqual(len(res), 1)

    def testEntryOnlyBody(self):
        res, _ = create_actions("entry/", "test", [], [])
        self.assertEqual(res[0].name, "entry")

    def testEmptyTrigger(self):
        res, _ = create_actions("TIME_TICK_10S/", "test", [], [])
        self.assertEqual(res[0].name, "TIME_TICK_10S")

    def testMoreSpacers(self):
        res, _ = create_actions("   \n   \nTIME_TICK_10S/   \n  ", "test", [], [])
        self.assertEqual(res[0].name, "TIME_TICK_10S")

    def testSeveralTriggers(self):
        res, _ = create_actions("""
                            entry/
                                BeepForPeriod(LONG_BEEP_MS);
                                SaveState(AGONY);
                                UpdateHP(me, 1);
                                me->TimerAgony = 0;

                            TIME_TICK_1S[else]/
                                me->TimerAgony++;
                                Flash(RED, 0, 0, FLASH_MS);

                            TIME_TICK_10S/
                                BeepForPeriod(SHORT_BEEP_MS);""", "test", [], [])

        self.assertEqual(len(res), 3)

    def testSeveralTriggersName(self):
        res, _ = create_actions("""
                            entry/
                                BeepForPeriod(LONG_BEEP_MS);
                                SaveState(AGONY);
                                UpdateHP(me, 1);
                                me->TimerAgony = 0;

                            TIME_TICK_1S[else]/
                                me->TimerAgony++;
                                Flash(RED, 0, 0, FLASH_MS);

                            TIME_TICK_10S/
                                BeepForPeriod(SHORT_BEEP_MS);""", "test", [], [])

        self.assertEqual(res[1].name, 'TIME_TICK_1S')

    def testSeveralTriggersBody(self):
        res, _ = create_actions("""
                            entry/
                                BeepForPeriod(LONG_BEEP_MS);
                                SaveState(AGONY);
                                UpdateHP(me, 1);
                                me->TimerAgony = 0;

                            TIME_TICK_1S[else]/
                                me->TimerAgony++;
                                Flash(RED, 0, 0, FLASH_MS);

                            TIME_TICK_10S/
                                BeepForPeriod(SHORT_BEEP_MS);""", "test", [], [])

        self.assertEqual(res[1].action, 'me->TimerAgony++;\nFlash(RED, 0, 0, FLASH_MS);')
        self.assertEqual(res[2].action, 'BeepForPeriod(SHORT_BEEP_MS);')

    def testGuard(self):
        res, _ = create_actions('RAD_RCVD[((((oregonPlayerQEvt*)e)->value+me->CharHP )<GHOUL_HP)]/\n    '
                                'UpdateHP(me, me->CharHP + ((oregonPlayerQEvt*)e)->value);;', "test", [], [])
        self.assertEqual(res[0].guard, "((((oregonPlayerQEvt*)e)->value+me->CharHP )<GHOUL_HP)")

    def testNamewithGuard(self):
        res, _ = create_actions('RAD_RCVD[((((oregonPlayerQEvt*)e)->value+me->CharHP )<GHOUL_HP)]/\n    '
                                'UpdateHP(me, me->CharHP + ((oregonPlayerQEvt*)e)->value);;', "test", [], [])
        self.assertEqual(res[0].guard, "((((oregonPlayerQEvt*)e)->value+me->CharHP )<GHOUL_HP)")

    def testActionwithGuard(self):
        res, _ = create_actions('RAD_RCVD[((((oregonPlayerQEvt*)e)->value+me->CharHP )<GHOUL_HP)]/\n    '
                                'UpdateHP(me, me->CharHP + ((oregonPlayerQEvt*)e)->value);;', "test", [], [])
        self.assertEqual(res[0].action, "UpdateHP(me, me->CharHP + ((oregonPlayerQEvt*)e)->value);;")

    def testElseGuard(self):
        res, _ = create_actions("""
                                    entry/
                                        BeepForPeriod(LONG_BEEP_MS);
                                        SaveState(AGONY);
                                        UpdateHP(me, 1);
                                        me->TimerAgony = 0;

                                    TIME_TICK_1S[else]/
                                        me->TimerAgony++;
                                        Flash(RED, 0, 0, FLASH_MS);

                                    TIME_TICK_10S/
                                        BeepForPeriod(SHORT_BEEP_MS);""", "test", [], [])
        self.assertEqual(res[1].guard, "else")

    def testNoTrigger(self):
        res, _ = create_actions("a = b / c", "test", [], [])
        self.assertEqual(len(res), 0)


if __name__ == '__main__':
    unittest.main()
