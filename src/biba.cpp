/*.$file${.::biba.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: biba.qm
* File:  ${.::biba.cpp}
*
*/
/*.$endhead${.::biba.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qhsm.h"
#include "biba.h"
#include "eventHandlers.h"
#include <stdint.h>
//Q_DEFINE_THIS_FILE
/* global-scope definitions -----------------------------------------*/
QHsm * const the_biba = (QHsm *) &biba; /* the opaque pointer */

/*.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*. Check for the minimum required QP version */
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 6.9.0 or higher required
#endif
/*.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::Biba_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Biba_ctor} .......................................................*/
void Biba_ctor(void) {
    Biba *me = &biba;
     
    QHsm_ctor(&me->super, Q_STATE_CAST(&Biba_initial));
}
/*.$enddef${SMs::Biba_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::Biba} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Biba} ............................................................*/
/*.${SMs::Biba::SM} ........................................................*/
QState Biba_initial(Biba * const me, void const * const par) {
    /*.${SMs::Biba::SM::initial} */

    return Q_TRAN(&Biba_On);
}
/*.${SMs::Biba::SM::global} ................................................*/
QState Biba_global(Biba * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {

#ifdef DESKTOP
        /*.${SMs::Biba::SM::global::TERMINATE} */
        case TERMINATE_SIG: {
            status_ = Q_TRAN(&Biba_final);
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
/*.${SMs::Biba::SM::global::On} ............................................*/
QState Biba_On(Biba * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Biba::SM::global::On} */
        case Q_ENTRY_SIG: {
            diod.turnOn();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Biba::SM::global::On} */
        case Q_EXIT_SIG: {

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Biba::SM::global::On::trig0} */
        case trig0_SIG: {
            /*.${SMs::Biba::SM::global::On::trig0::[btn.isJustPressed()]} */
            if (btn.isJustPressed()) {
                status_ = Q_TRAN(&Biba_Off);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Biba_global);
            break;
        }
    }
    return status_;
}
/*.${SMs::Biba::SM::global::Off} ...........................................*/
QState Biba_Off(Biba * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Biba::SM::global::Off} */
        case Q_ENTRY_SIG: {
            diod.turnOff();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Biba::SM::global::Off} */
        case Q_EXIT_SIG: {

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Biba::SM::global::Off::trig1} */
        case trig1_SIG: {
            /*.${SMs::Biba::SM::global::Off::trig1::[btn.isJustPressed()]} */
            if (btn.isJustPressed()) {
                status_ = Q_TRAN(&Biba_On);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Biba_global);
            break;
        }
    }
    return status_;
}

#ifdef DESKTOP
/*.${SMs::Biba::SM::final} .........................................*/
QState Biba_final(Biba * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Biba::SM::final} */
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

/*.$enddef${SMs::Biba} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/*tranlated from diagrams code*/

//Start of c code from diagram
Button btn = Button(10);
Led diod = Led(13);
//End of c code from diagram


