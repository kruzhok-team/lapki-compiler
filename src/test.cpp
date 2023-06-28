/*.$file${.::test.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: test.qm
* File:  ${.::test.cpp}
*
*/
/*.$endhead${.::test.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qhsm.h"
#include "test.h"
#include "eventHandlers.h"
#include <stdint.h>
//Q_DEFINE_THIS_FILE
/* global-scope definitions -----------------------------------------*/
QHsm * const the_test = (QHsm *) &test; /* the opaque pointer */

/*.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*. Check for the minimum required QP version */
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 6.9.0 or higher required
#endif
/*.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::Test_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Test_ctor} .......................................................*/
void Test_ctor(void) {
    Test *me = &test;
     
    QHsm_ctor(&me->super, Q_STATE_CAST(&Test_initial));
}
/*.$enddef${SMs::Test_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::Test} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Test} ............................................................*/
/*.${SMs::Test::SM} ........................................................*/
QState Test_initial(Test * const me, void const * const par) {
    /*.${SMs::Test::SM::initial} */

    return Q_TRAN(&Test_init);
}
/*.${SMs::Test::SM::init} ..................................................*/
QState Test_init(Test * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Test::SM::init} */
        case Q_ENTRY_SIG: {

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Test::SM::init} */
        case Q_EXIT_SIG: {

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
/*.${SMs::Test::SM::init::Off} .............................................*/
QState Test_Off(Test * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Test::SM::init::Off} */
        case Q_ENTRY_SIG: {
            diod.turnOff();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Test::SM::init::Off} */
        case Q_EXIT_SIG: {

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Test::SM::init::Off::Trig1} */
        case Trig1_SIG: {
            /*.${SMs::Test::SM::init::Off::Trig1::[btn.isJustPressed()]} */
            if (btn.isJustPressed()) {
                status_ = Q_TRAN(&Test_On);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
/*.${SMs::Test::SM::init::On} ..............................................*/
QState Test_On(Test * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Test::SM::init::On} */
        case Q_ENTRY_SIG: {
            diod.turnOn();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Test::SM::init::On} */
        case Q_EXIT_SIG: {

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Test::SM::init::On::Trig2} */
        case Trig2_SIG: {
            /*.${SMs::Test::SM::init::On::Trig2::[btn.isJustPressed()]} */
            if (btn.isJustPressed()) {
                status_ = Q_TRAN(&Test_Off);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}

#ifdef DESKTOP
/*.${SMs::Test::SM::final} .........................................*/
QState Test_final(Test * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Test::SM::final} */
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

/*.$enddef${SMs::Test} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/*tranlated from diagrams code*/

//Start of c code from diagram
Led diod = Led(12);
//End of c code from diagram


