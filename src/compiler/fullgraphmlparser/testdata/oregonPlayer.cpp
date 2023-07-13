/*.$file${.::oregonPlayer.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: oregonPlayer.qm
* File:  ${.::oregonPlayer.cpp}
*
*/
/*.$endhead${.::oregonPlayer.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qhsm.h"
#include "oregonPlayer.h"
#include "eventHandlers.h"
#include <stdint.h>
//Q_DEFINE_THIS_FILE
/* global-scope definitions -----------------------------------------*/
QHsm * const the_oregonPlayer = (QHsm *) &oregonPlayer; /* the opaque pointer */

/*.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*. Check for the minimum required QP version */
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 6.9.0 or higher required
#endif
/*.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::OregonPlayer_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::OregonPlayer_ctor} ...............................................*/
void OregonPlayer_ctor(
    unsigned int HP,
    unsigned int State,
    unsigned int TimerAgony)
{
    OregonPlayer *me = &oregonPlayer;
     me->CharHP = HP;
    me->TimerAgony = TimerAgony;
    switch (State) {
               case HEALTHY: {
                   me->StartState =
                   (QStateHandler)&OregonPlayer_healthy;
                   break;
               }
               case AGONY: {
                   me->StartState =
                   (QStateHandler)& OregonPlayer_agony;
                   break;
               }
               case DEAD: {
                   me->StartState =
                   (QStateHandler)& OregonPlayer_dead;
                   break;

               }
               case GHOUL_GOOD: {
                        me->StartState =
                        (QStateHandler)& OregonPlayer_ghoul_good;
                        break;
                    }
               case GHOUL_WOUNDED: {
                        me->StartState =
                        (QStateHandler)& OregonPlayer_wounded;
                        break;
                    }
               case GHOUL_HEALING: {
                        me->StartState =
                        (QStateHandler)& OregonPlayer_ghoul_healing;
                        break;
                    }
               case BLESSED: {
                        me->StartState =
                        (QStateHandler)& OregonPlayer_blessed;
                        break;
                    }
               default:
                   me->StartState =(QStateHandler)& OregonPlayer_healthy;
           }
    QHsm_ctor(&me->super, Q_STATE_CAST(&OregonPlayer_initial));
}
/*.$enddef${SMs::OregonPlayer_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::OregonPlayer} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::OregonPlayer} ....................................................*/
/*.${SMs::OregonPlayer::SM} ................................................*/
QState OregonPlayer_initial(OregonPlayer * const me, void const * const par) {
    /*.${SMs::OregonPlayer::SM::initial} */
    return Q_TRAN(me->StartState);
    return Q_TRAN(&OregonPlayer_healthy);
}
/*.${SMs::OregonPlayer::SM::global} ........................................*/
QState OregonPlayer_global(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {

#ifdef DESKTOP
        /*.${SMs::OregonPlayer::SM::global::TERMINATE} */
        case TERMINATE_SIG: {
            status_ = Q_TRAN(&OregonPlayer_final);
            break;
        }
#endif /* def DESKTOP */

        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active} ................................*/
QState OregonPlayer_active(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state active");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state active");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_global);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active::alive} .........................*/
QState OregonPlayer_alive(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active::alive} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state alive");
            #endif /* def DESKTOP */
            
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state alive");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_active);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active::alive::immune} .................*/
QState OregonPlayer_immune(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active::alive::immune} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state immune");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::immune} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state immune");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::immune::TIME_TICK_1S} */
        case TIME_TICK_1S_SIG: {
            Flash(0, GREEN_MEDIUM, 0, FLASH_SEC);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::immune::HEAL} */
        case HEAL_SIG: {
            UpdateHP(me, me->CharHP+((oregonPlayerQEvt*)e)->value);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::immune::AGONY} */
        case AGONY_SIG: {
            status_ = Q_TRAN(&OregonPlayer_agony);
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_alive);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active::alive::immune::temp_immune} ....*/
QState OregonPlayer_temp_immune(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active::alive::immune::temp_immune} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state temp_immune");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::immune::temp_immune} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state temp_immune");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::immune::temp_immune::NOT_IMMUNE} */
        case NOT_IMMUNE_SIG: {
            status_ = Q_TRAN(&OregonPlayer_healthy);
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_immune);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active::alive::immune::blessed} ........*/
QState OregonPlayer_blessed(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active::alive::immune::blessed} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state blessed");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::immune::blessed} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state blessed");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_immune);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active::alive::healthy} ................*/
QState OregonPlayer_healthy(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active::alive::healthy} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state healthy");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::healthy} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state healthy");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::healthy::TIME_TICK_1S} */
        case TIME_TICK_1S_SIG: {
            ShowCurrentHealth(me);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::healthy::HEAL} */
        case HEAL_SIG: {
            UpdateHP(me, me->CharHP+((oregonPlayerQEvt*)e)->value);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::healthy::RAD_RCVD} */
        case RAD_RCVD_SIG: {
            /*.${SMs::OregonPlayer::SM::global::active::alive::healthy::RAD_RCVD::[((oregonPlayerQEvt*)e)->value>=~} */
            if (((oregonPlayerQEvt*)e)->value >= me->CharHP) {
                status_ = Q_TRAN(&OregonPlayer_agony);
            }
            /*.${SMs::OregonPlayer::SM::global::active::alive::healthy::RAD_RCVD::[else]} */
            else {
                UpdateHP(me, me->CharHP-((oregonPlayerQEvt*)e)->value);
                status_ = Q_HANDLED();
            }
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::healthy::AGONY} */
        case AGONY_SIG: {
            status_ = Q_TRAN(&OregonPlayer_agony);
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::healthy::IMMUNE} */
        case IMMUNE_SIG: {
            status_ = Q_TRAN(&OregonPlayer_temp_immune);
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_alive);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active::alive::agony} ..................*/
QState OregonPlayer_agony(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active::alive::agony} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state agony");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::agony} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state agony");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::agony::TIME_TICK_10S} */
        case TIME_TICK_10S_SIG: {
            BeepForPeriod(SHORT_BEEP_MS);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::agony::TIME_TICK_1S} */
        case TIME_TICK_1S_SIG: {
            /*.${SMs::OregonPlayer::SM::global::active::alive::agony::TIME_TICK_1S::[me->TimerAgony>TIMEOUT_AGONY_S]} */
            if (me->TimerAgony > TIMEOUT_AGONY_S) {
                status_ = Q_TRAN(&OregonPlayer_dead);
            }
            /*.${SMs::OregonPlayer::SM::global::active::alive::agony::TIME_TICK_1S::[else]} */
            else {
                me->TimerAgony++;
                Flash(RED, 0, 0, FLASH_MS);
                status_ = Q_HANDLED();
            }
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::agony::BLESSED} */
        case BLESSED_SIG: {
            status_ = Q_TRAN(&OregonPlayer_blessed);
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::alive::agony::HEAL} */
        case HEAL_SIG: {
            UpdateHP(me, me->CharHP + ((oregonPlayerQEvt*)e)->value);
            status_ = Q_TRAN(&OregonPlayer_healthy);
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_alive);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active::ghoul} .........................*/
QState OregonPlayer_ghoul(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active::ghoul} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state ghoul");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::ghoul} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state ghoul");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::ghoul::TIME_TICK_1S} */
        case TIME_TICK_1S_SIG: {
            ShowCurrentHealthGhoul(me);
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_active);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active::ghoul::ghoul_good} .............*/
QState OregonPlayer_ghoul_good(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active::ghoul::ghoul_good} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state ghoul_good");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::ghoul::ghoul_good} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state ghoul_good");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_ghoul);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active::ghoul::ghoul_healing} ..........*/
QState OregonPlayer_ghoul_healing(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active::ghoul::ghoul_healing} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state ghoul_healing");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::ghoul::ghoul_healing} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state ghoul_healing");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::ghoul::ghoul_healing::RAD_RCVD} */
        case RAD_RCVD_SIG: {
            /*.${SMs::OregonPlayer::SM::global::active::ghoul::ghoul_healing::RAD_RCVD::[((((oregonPlayerQEvt*)e)->value~} */
            if (((((oregonPlayerQEvt*)e)->value+me->CharHP )>=GHOUL_HP)) {
                status_ = Q_TRAN(&OregonPlayer_ghoul_good);
            }
            /*.${SMs::OregonPlayer::SM::global::active::ghoul::ghoul_healing::RAD_RCVD::[else]} */
            else {
                UpdateHP(me, me->CharHP + ((oregonPlayerQEvt*)e)->value);
                status_ = Q_HANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_ghoul);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active::ghoul::wounded} ................*/
QState OregonPlayer_wounded(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active::ghoul::wounded} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state wounded");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::ghoul::wounded} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state wounded");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::ghoul::wounded::TIME_TICK_10S} */
        case TIME_TICK_10S_SIG: {
            BeepForPeriod(SHORT_BEEP_MS);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::ghoul::wounded::RAD_RCVD} */
        case RAD_RCVD_SIG: {
            UpdateHP(me, me->CharHP + ((oregonPlayerQEvt*)e)->value);
            status_ = Q_TRAN(&OregonPlayer_ghoul_healing);
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_ghoul);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::active::dead} ..........................*/
QState OregonPlayer_dead(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::active::dead} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state dead");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::dead} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state dead");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::active::dead::TIME_TICK_1M} */
        case TIME_TICK_1M_SIG: {
            BeepForPeriod(SHORT_BEEP_MS);
            Flash(255, 0, 0, FLASH_1M);
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_active);
            break;
        }
    }
    return status_;
}
/*.${SMs::OregonPlayer::SM::global::test} ..................................*/
QState OregonPlayer_test(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::global::test} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state test");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::test} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state test");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::OregonPlayer::SM::global::test::RAD_RCVD} */
        case RAD_RCVD_SIG: {
            BeepForPeriod(SHORT_BEEP_MS);
            Flash(127, 0, 0, FLASH_MS);
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&OregonPlayer_global);
            break;
        }
    }
    return status_;
}

#ifdef DESKTOP
/*.${SMs::OregonPlayer::SM::final} .........................................*/
QState OregonPlayer_final(OregonPlayer * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::OregonPlayer::SM::final} */
        case Q_ENTRY_SIG: {
            printf("
            Bye! Bye!
            "); exit(0);
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
#endif /* def DESKTOP */

/*.$enddef${SMs::OregonPlayer} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/*tranlated from diagrams code*/
