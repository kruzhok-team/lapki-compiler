/*.$file${.::ability.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: ability.qm
* File:  ${.::ability.cpp}
*
*/
/*.$endhead${.::ability.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qhsm.h"
#include "ability.h"
#include <stdint.h>
//Q_DEFINE_THIS_FILE
/* global-scope definitions -----------------------------------------*/
QHsm * const the_ability = (QHsm *) &ability; /* the opaque pointer *
/*.$define${SMs::Ability_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Ability_ctor} ....................................................*/
void Ability_ctor(
    unsigned int ability_pause,
    unsigned int ability)
{
    Ability *me = &ability;
         me->ability_pause = ability_pause;
        me->count = 0;
        me->ability = ability;
    QHsm_ctor(&me->super, Q_STATE_CAST(&Ability_initial));
}
/*.$enddef${SMs::Ability_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::Ability} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Ability} .........................................................*/
/*.${SMs::Ability::SM} .....................................................*/
QState Ability_initial(Ability * const me, void const * const par) {
    /*.${SMs::Ability::SM::initial} */

    return Q_TRAN(&Ability_idle);
}
/*.${SMs::Ability::SM::global} .............................................*/
QState Ability_global(Ability * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {

#ifdef DESKTOP
        /*.${SMs::Ability::SM::global::TERMINATE} */
        case TERMINATE_SIG: {
            status_ = Q_TRAN(&Ability_final);
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
/*.${SMs::Ability::SM::global::ability} ....................................*/
QState Ability_ability(Ability * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Ability::SM::global::ability} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state ability");
            #endif /* def DESKTOP */
            FlashAbilityColor();
            me->ability_pause = 0;
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Ability::SM::global::ability} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state ability");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Ability_global);
            break;
        }
    }
    return status_;
}
/*.${SMs::Ability::SM::global::ability::idle} ..............................*/
QState Ability_idle(Ability * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Ability::SM::global::ability::idle} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state idle");
            #endif /* def DESKTOP */
            SavePause();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Ability::SM::global::ability::idle} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state idle");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Ability::SM::global::ability::idle::TIME_TICK_1M} */
        case TIME_TICK_1M_SIG: {
            /*.${SMs::Ability::SM::global::ability::idle::TIME_TICK_1M::[me->ability_pause>0]} */
            if (me->ability_pause > 0) {
                me->ability_pause--;
                SavePause();
                status_ = Q_HANDLED();
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        /*.${SMs::Ability::SM::global::ability::idle::PILL_ABILITY} */
        case PILL_ABILITY_SIG: {
            me->ability = e->id;
            FlashAbilityColor();
            SaveAbility(me->ability);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Ability::SM::global::ability::idle::LONG_PRESS_THIRD} */
        case LONG_PRESS_THIRD_SIG: {
            /*.${SMs::Ability::SM::global::ability::idle::LONG_PRESS_THIRD::[me->ability_pause==0]} */
            if (me->ability_pause == 0) {
                status_ = Q_TRAN(&Ability_active);
            }
            /*.${SMs::Ability::SM::global::ability::idle::LONG_PRESS_THIRD::[else]} */
            else {
                FlashWrong();
                status_ = Q_HANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Ability_ability);
            break;
        }
    }
    return status_;
}
/*.${SMs::Ability::SM::global::ability::active} ............................*/
QState Ability_active(Ability * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Ability::SM::global::ability::active} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state active");
            #endif /* def DESKTOP */
            StartAbility();
            me->count = 0;
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Ability::SM::global::ability::active} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state active");
            #endif /* def DESKTOP */
            me->ability_pause = ABILITY_PAUSE_M;
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Ability::SM::global::ability::active::TIME_TICK_1S} */
        case TIME_TICK_1S_SIG: {
            /*.${SMs::Ability::SM::global::ability::active::TIME_TICK_1S::[me->count>=ABILITY_THRESHOLD_1S]} */
            if (me->count >= ABILITY_THRESHOLD_1S) {
                status_ = Q_TRAN(&Ability_idle);
            }
            /*.${SMs::Ability::SM::global::ability::active::TIME_TICK_1S::[else]} */
            else {
                me->count++;
                status_ = Q_HANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Ability_ability);
            break;
        }
    }
    return status_;
}

#ifdef DESKTOP
/*.${SMs::Ability::SM::final} .........................................*/
QState Ability_final(Ability * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Ability::SM::final} */
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

/*.$enddef${SMs::Ability} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/*tranlated from diagrams code*/
