/*.$file${.::player_type.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: player_type.qm
* File:  ${.::player_type.cpp}
*
*/
/*.$endhead${.::player_type.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qhsm.h"
#include "player_type.h"
#include "eventHandlers.h"
#include <stdint.h>
//Q_DEFINE_THIS_FILE
/* global-scope definitions -----------------------------------------*/
QHsm * const the_player_type = (QHsm *) &player_type; /* the opaque pointer */

/*.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*. Check for the minimum required QP version */
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 6.9.0 or higher required
#endif
/*.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::Player_type_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Player_type_ctor} ................................................*/
void Player_type_ctor(
    Health* CharHealth,
    unsigned int TimeRegen,
    unsigned int State)
{
    Player_type *me = &player_type;

     me->TimeRegen = 0;
     me->CharHealth = Health;
        switch (State) {
            case DEAD: {
                me->StartState =
                (QStateHandler)&Player_type_dead;
                break;
            }
            case LOCAL: {
                me->StartState =
                (QStateHandler)&Player_type_regenerating;
                break;
            }
            case TAILOR: {
                me->StartState =
                (QStateHandler)&Player_type_tailor;
                break;

            }
            case STALKER: {
                me->StartState =
                (QStateHandler)&Player_type_stalker;
                break;
            }
            default:
                me->StartState =
                (QStateHandler)&Player_type_dead;
        }
    QHsm_ctor(&me->super, Q_STATE_CAST(&Player_type_initial));
}
/*.$enddef${SMs::Player_type_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::Player_type} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Player_type} .....................................................*/
/*.${SMs::Player_type::SM} .................................................*/
QState Player_type_initial(Player_type * const me, void const * const par) {
    /*.${SMs::Player_type::SM::initial} */
    return Q_TRAN(me->StartState);
    return Q_TRAN(&Player_type_dead);
}
/*.${SMs::Player_type::SM::global} .........................................*/
QState Player_type_global(Player_type * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {

#ifdef DESKTOP
        /*.${SMs::Player_type::SM::global::TERMINATE} */
        case TERMINATE_SIG: {
            status_ = Q_TRAN(&Player_type_final);
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
/*.${SMs::Player_type::SM::global::player_type} ............................*/
QState Player_type_player_type(Player_type * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Player_type::SM::global::player_type} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state player_type");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state player_type");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::PILL_TAILOR} */
        case PILL_TAILOR_SIG: {
            status_ = Q_TRAN(&Player_type_tailor);
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::PILL_STALKER} */
        case PILL_STALKER_SIG: {
            UpdateMaxHP(me->CharHealth, StalkerHP);
                PlayerType_Save(STALKER);
            status_ = Q_TRAN(&Player_type_regenerating);
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::PILL_LOCAL} */
        case PILL_LOCAL_SIG: {
            status_ = Q_TRAN(&Player_type_regenerating);
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::PILL_RESET} */
        case PILL_RESET_SIG: {
            status_ = Q_TRAN(&Player_type_normal);
            break;
        }
        default: {
            status_ = Q_SUPER(&Player_type_global);
            break;
        }
    }
    return status_;
}
/*.${SMs::Player_type::SM::global::player_type::alive} .....................*/
QState Player_type_alive(Player_type * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Player_type::SM::global::player_type::alive} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state alive");
            #endif /* def DESKTOP */
            SIMPLE_DISPATCH(the_health, RESET);
            SIMPLE_DISPATCH(the_ability, RESET);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::alive} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state alive");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::alive::PILL_RESET} */
        case PILL_RESET_SIG: {
            SIMPLE_DISPATCH(the_health, RESET);
            IMPLE_DISPATCH(the_ability, RESET);
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Player_type_player_type);
            break;
        }
    }
    return status_;
}
/*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate} .....*/
QState Player_type_may_regenerate(Player_type * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state may_regenerate");
            #endif /* def DESKTOP */
            UpdateMaxHP(me->CharHealth; LocalHP);
            Flash(0, 0, 255, FLASH_MS);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state may_regenerate");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Player_type_alive);
            break;
        }
    }
    return status_;
}
/*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::normal} */
QState Player_type_normal(Player_type * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::normal} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state normal");
            #endif /* def DESKTOP */
            TimerRegen_Update(me, 0);
            PlayerType_Save(LOCAL);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::normal} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state normal");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::normal::DMG_RCVD} */
        case DMG_RCVD_SIG: {
            TimerRegen_Update(me, 0);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::normal::TIME_TICK_1S} */
        case TIME_TICK_1S_SIG: {
            /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::normal::TIME_TICK_1S::[me->TimerRegen>RegenThresh]} */
            if (me->TimerRegen > RegenThresh) {
                status_ = Q_TRAN(&Player_type_regenerating);
            }
            /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::normal::TIME_TICK_1S::[else]} */
            else {
                TimerRegen_Update(me, TimerRegen+1);
                status_ = Q_HANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Player_type_may_regenerate);
            break;
        }
    }
    return status_;
}
/*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::regenerating} */
QState Player_type_regenerating(Player_type * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::regenerating} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state regenerating");
            #endif /* def DESKTOP */
            PlayerType_Save(LOCAL);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::regenerating} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state regenerating");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::regenerating::TIME_TICK_1S} */
        case TIME_TICK_1S_SIG: {
            playerTypeQEvt new_e;
            new_e.super.sig = HEAL_SIG;
            new_e.Value = HealAmount;
            QMSM_DISPATCH(the_health, (QEvt *)&new_e);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::alive::may_regenerate::regenerating::DMG_RCVD} */
        case DMG_RCVD_SIG: {
            status_ = Q_TRAN(&Player_type_normal);
            break;
        }
        default: {
            status_ = Q_SUPER(&Player_type_may_regenerate);
            break;
        }
    }
    return status_;
}
/*.${SMs::Player_type::SM::global::player_type::alive::tailor} .............*/
QState Player_type_tailor(Player_type * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Player_type::SM::global::player_type::alive::tailor} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state tailor");
            #endif /* def DESKTOP */
            UpdateMaxHP(me->CharHealth, TailorHP);
            Flash(255, 255, 0, FLASH_MS);
            PlayerType_Save(TAILOR);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::alive::tailor} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state tailor");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Player_type_alive);
            break;
        }
    }
    return status_;
}
/*.${SMs::Player_type::SM::global::player_type::alive::stalker} ............*/
QState Player_type_stalker(Player_type * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Player_type::SM::global::player_type::alive::stalker} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state stalker");
            #endif /* def DESKTOP */
            UpdateMaxHP(me->CharHealth, StalkerHP);
            Flash(255, 0, 0, FLASH_MS);
            PlayerType_Save(STALKER);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::alive::stalker} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state stalker");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Player_type_alive);
            break;
        }
    }
    return status_;
}
/*.${SMs::Player_type::SM::global::player_type::dead} ......................*/
QState Player_type_dead(Player_type * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Player_type::SM::global::player_type::dead} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state dead");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Player_type::SM::global::player_type::dead} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state dead");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Player_type_player_type);
            break;
        }
    }
    return status_;
}

#ifdef DESKTOP
/*.${SMs::Player_type::SM::final} ..........................................*/
QState Player_type_final(Player_type * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Player_type::SM::final} */
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

/*.$enddef${SMs::Player_type} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/*tranlated from diagrams code*/

//Start of c code from diagram
//stub code
unsigned int TailorHP = TAILOR_HP;
unsigned int LocalHP = LOCAL_HP;
unsigned int StalkerHP = STALKER_HP;
//End of c code from diagram
