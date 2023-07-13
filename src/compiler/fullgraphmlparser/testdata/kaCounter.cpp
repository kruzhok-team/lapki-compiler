/*.$file${.::kaCounter.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: kaCounter.qm
* File:  ${.::kaCounter.cpp}
*
*/
/*.$endhead${.::kaCounter.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qhsm.h"
#include "kaCounter.h"
#include "eventHandlers.h"
#include <stdint.h>
//Q_DEFINE_THIS_FILE
/* global-scope definitions -----------------------------------------*/
QHsm * const the_kaCounter = (QHsm *) &kaCounter; /* the opaque pointer */

/*.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*. Check for the minimum required QP version */
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 6.9.0 or higher required
#endif
/*.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::KaCounter_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::KaCounter_ctor} ..................................................*/
void KaCounter_ctor(void) {
    KaCounter *me = &kaCounter;

    QHsm_ctor(&me->super, Q_STATE_CAST(&KaCounter_initial));
}
/*.$enddef${SMs::KaCounter_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::KaCounter} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::KaCounter} .......................................................*/
/*.${SMs::KaCounter::SM} ...................................................*/
QState KaCounter_initial(KaCounter * const me, void const * const par) {
    /*.${SMs::KaCounter::SM::initial} */
    return Q_TRAN(&KaCounter_idle);
}
/*.${SMs::KaCounter::SM::global} ...........................................*/
QState KaCounter_global(KaCounter * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {

#ifdef DESKTOP
        /*.${SMs::KaCounter::SM::global::TERMINATE} */
        case TERMINATE_SIG: {
            status_ = Q_TRAN(&KaCounter_final);
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
/*.${SMs::KaCounter::SM::global::ka_tet_counter} ...........................*/
QState KaCounter_ka_tet_counter(KaCounter * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::KaCounter::SM::global::ka_tet_counter} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state ka_tet_counter");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaCounter::SM::global::ka_tet_counter} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state ka_tet_counter");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaCounter::SM::global::ka_tet_counter::TIME_TICK_1M} */
        case TIME_TICK_1M_SIG: {
            SaveCounters(me->KaTetTimers);
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&KaCounter_global);
            break;
        }
    }
    return status_;
}
/*.${SMs::KaCounter::SM::global::ka_tet_counter::idle} .....................*/
QState KaCounter_idle(KaCounter * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::KaCounter::SM::global::ka_tet_counter::idle} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state idle");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaCounter::SM::global::ka_tet_counter::idle} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state idle");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaCounter::SM::global::ka_tet_counter::idle::BEGIN(PERSON_NEAR)} */
        case BEGIN(PERSON_NEAR)_SIG: {
            /*.${SMs::KaCounter::SM::global::ka_tet_counter::idle::BEGIN(PERSON_NEA~::[(((me->KaTets)->get(((constKaCo~} */
            if ((((me->KaTets)->get(((const KaCounterQEvt*)e)->id)) == false) && (me->Characters->count_ones() ==1))) {
                me->CurrentId = ((const KaCounterQEvt*)e)->id;
                status_ = Q_TRAN(&KaCounter_ka_tet_forming);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        /*.${SMs::KaCounter::SM::global::ka_tet_counter::idle::END(PERSON_NEAR)} */
        case END(PERSON_NEAR)_SIG: {
            /*.${SMs::KaCounter::SM::global::ka_tet_counter::idle::END(PERSON_NEAR)::[(me->Characters->count_ones()==~} */
            if ((me->Characters->count_ones() == 1) && (me->KaTets)->get(me->Characters->find_first_one()) == false)) {
                me->CurrentId = me->Characters->find_first_one();
                status_ = Q_TRAN(&KaCounter_ka_tet_forming);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&KaCounter_ka_tet_counter);
            break;
        }
    }
    return status_;
}
/*.${SMs::KaCounter::SM::global::ka_tet_counter::ka_tet_forming} ...........*/
QState KaCounter_ka_tet_forming(KaCounter * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::KaCounter::SM::global::ka_tet_counter::ka_tet_forming} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state ka_tet_forming");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaCounter::SM::global::ka_tet_counter::ka_tet_forming} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state ka_tet_forming");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaCounter::SM::global::ka_tet_counter::ka_tet_forming::END(PERSON_NEAR)} */
        case END(PERSON_NEAR)_SIG: {
            /*.${SMs::KaCounter::SM::global::ka_tet_counter::ka_tet_forming::END(PERSON_NEAR)::[((constKaCounterQEvt*)e)->id==m~} */
            if (((const KaCounterQEvt*)e)->id == me->CurrentId) {
                status_ = Q_TRAN(&KaCounter_idle);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        /*.${SMs::KaCounter::SM::global::ka_tet_counter::ka_tet_forming::BEGIN(PERSON_NEAR)} */
        case BEGIN(PERSON_NEAR)_SIG: {
            /*.${SMs::KaCounter::SM::global::ka_tet_counter::ka_tet_forming::BEGIN(PERSON_NEA~::[((constKaCounterQEvt*)e)->id!=m~} */
            if (((const KaCounterQEvt*)e)->id != me->CurrentId) {
                status_ = Q_TRAN(&KaCounter_idle);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        /*.${SMs::KaCounter::SM::global::ka_tet_counter::ka_tet_forming::TIME_TICK_1S} */
        case TIME_TICK_1S_SIG: {
            /*.${SMs::KaCounter::SM::global::ka_tet_counter::ka_tet_forming::TIME_TICK_1S::[((*(me->KaTetTimers))[me->Curre~} */
            if (((*(me->KaTetTimers))[me->CurrentId] >= KATET_THRESHOLD)) {
                DISPATCH_ONESHOT_WITH_PARAM(FORM_KATET,  me->CurrentId);
                status_ = Q_TRAN(&KaCounter_idle);
            }
            /*.${SMs::KaCounter::SM::global::ka_tet_counter::ka_tet_forming::TIME_TICK_1S::[else]} */
            else {
                (*(me->KaTetTimers))[me->CurrentId]++;
                status_ = Q_HANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&KaCounter_ka_tet_counter);
            break;
        }
    }
    return status_;
}

#ifdef DESKTOP
/*.${SMs::KaCounter::SM::final} ............................................*/
QState KaCounter_final(KaCounter * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::KaCounter::SM::final} */
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

/*.$enddef${SMs::KaCounter} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/*tranlated from diagrams code*/
