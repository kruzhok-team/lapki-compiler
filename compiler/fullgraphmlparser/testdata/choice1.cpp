/*.$file${.::choice1.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: choice1.qm
* File:  ${.::choice1.cpp}
*
*/
/*.$endhead${.::choice1.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qhsm.h"
#include "choice1.h"
// #include "eventHandlers.h"
#include <stdint.h>
//Q_DEFINE_THIS_FILE
/* global-scope definitions -----------------------------------------*/
QHsm * const the_choice1 = (QHsm *) &choice1; /* the opaque pointer */

/*.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*. Check for the minimum required QP version */
// #if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
// #error qpc version 6.9.0 or higher required
// #endif
/*.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::Choice1_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Choice1_ctor} ....................................................*/
void Choice1_ctor(void) {
    Choice1 *me = &choice1;

    QHsm_ctor(&me->super, Q_STATE_CAST(&Choice1_initial));
}
/*.$enddef${SMs::Choice1_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::Choice1} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Choice1} .........................................................*/
/*.${SMs::Choice1::SM} .....................................................*/
QState Choice1_initial(Choice1 * const me, void const * const par) {
    /*.${SMs::Choice1::SM::initial} */
    return Q_TRAN(&Choice1_idle);
}
/*.${SMs::Choice1::SM::global} .............................................*/
QState Choice1_global(Choice1 * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {

#ifdef DESKTOP
        /*.${SMs::Choice1::SM::global::TERMINATE} */
        case TERMINATE_SIG: {
            status_ = Q_TRAN(&Choice1_final);
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
/*.${SMs::Choice1::SM::global::parent} .....................................*/
QState Choice1_parent(Choice1 * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Choice1::SM::global::parent} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state parent");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Choice1::SM::global::parent} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state parent");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Choice1_global);
            break;
        }
    }
    return status_;
}
/*.${SMs::Choice1::SM::global::parent::idle} ...............................*/
QState Choice1_idle(Choice1 * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Choice1::SM::global::parent::idle} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state idle");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Choice1::SM::global::parent::idle} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state idle");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Choice1::SM::global::parent::idle::TRIGGER} */
        case TRIGGER_SIG: {
            /*.${SMs::Choice1::SM::global::parent::idle::TRIGGER::[guard]} */
            if (guard) {
                do something
                status_ = Q_TRAN(&Choice1_choice1);
            }
            /*.${SMs::Choice1::SM::global::parent::idle::TRIGGER::[else]} */
            else {
                do something
                status_ = Q_TRAN(&Choice1_choice2);
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Choice1_parent);
            break;
        }
    }
    return status_;
}
/*.${SMs::Choice1::SM::global::choice1} ....................................*/
QState Choice1_choice1(Choice1 * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Choice1::SM::global::choice1} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state choice1");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Choice1::SM::global::choice1} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state choice1");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Choice1_global);
            break;
        }
    }
    return status_;
}
/*.${SMs::Choice1::SM::global::choice2} ....................................*/
QState Choice1_choice2(Choice1 * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Choice1::SM::global::choice2} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state choice2");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Choice1::SM::global::choice2} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state choice2");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Choice1_global);
            break;
        }
    }
    return status_;
}

#ifdef DESKTOP
/*.${SMs::Choice1::SM::final} ..............................................*/
QState Choice1_final(Choice1 * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Choice1::SM::final} */
        case Q_ENTRY_SIG: {
            printf("Bye! Bye!"); 
            exit(0);
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

/*.$enddef${SMs::Choice1} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/*tranlated from diagrams code*/
