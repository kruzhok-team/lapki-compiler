/*.$file${.::kaTet.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: kaTet.qm
* File:  ${.::kaTet.cpp}
*
*/
/*.$endhead${.::kaTet.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qhsm.h"
#include "kaTet.h"
#include "eventHandlers.h"
#include <stdint.h>
//Q_DEFINE_THIS_FILE
/* global-scope definitions -----------------------------------------*/
QHsm * const the_kaTet = (QHsm *) &kaTet; /* the opaque pointer */

/*.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*. Check for the minimum required QP version */
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 6.9.0 or higher required
#endif
/*.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::KaTet_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::KaTet_ctor} ......................................................*/
void KaTet_ctor(void) {
    KaTet *me = &kaTet;

    QHsm_ctor(&me->super, Q_STATE_CAST(&KaTet_initial));
}
/*.$enddef${SMs::KaTet_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::KaTet} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::KaTet} ...........................................................*/
/*.${SMs::KaTet::SM} .......................................................*/
QState KaTet_initial(KaTet * const me, void const * const par) {
    /*.${SMs::KaTet::SM::initial} */
    return Q_TRAN(me->StartState);
    return Q_TRAN(&KaTet_alone);
}
/*.${SMs::KaTet::SM::global} ...............................................*/
QState KaTet_global(KaTet * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {

#ifdef DESKTOP
        /*.${SMs::KaTet::SM::global::TERMINATE} */
        case TERMINATE_SIG: {
            status_ = Q_TRAN(&KaTet_final);
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
/*.${SMs::KaTet::SM::global::has_katet} ....................................*/
QState KaTet_has_katet(KaTet * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::KaTet::SM::global::has_katet} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state has_katet");
            #endif /* def DESKTOP */
            SaveKatet(me->KaTets);
            me->KaTetNear = 1;
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaTet::SM::global::has_katet} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state has_katet");
            #endif /* def DESKTOP */
            ScreenAddBMPToQueue("Katet_destroyed.bmp");

            N(FORM_KATET)/
            (me->KaTets)->set(((const KaTetQEvt*)e)->parameters, true);
            ScreenAddBMPToQueue("Katet.bmp");
            Vibro(MEDIUM_VIBRO);
            SaveKatet(me->KaTets);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaTet::SM::global::has_katet::BEGIN(DESTROY_KATET)} */
        case BEGIN(DESTROY_KATET)_SIG: {
            (me->KaTets)->clear();
                SaveKatet(me->KaTets);
                DISPATCH_ONESHOT(KATET_DESTROYED);
            status_ = Q_TRAN(&KaTet_alone);
            break;
        }
        default: {
            status_ = Q_SUPER(&KaTet_global);
            break;
        }
    }
    return status_;
}
/*.${SMs::KaTet::SM::global::has_katet::near} ..............................*/
QState KaTet_near(KaTet * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::KaTet::SM::global::has_katet::near} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state near");
            #endif /* def DESKTOP */
            DISPATCH_BEGIN(KATET_NEAR);

            N(PERSON_NEAR)/
            if ((me->KaTets)->get(((const KaTetQEvt*)e)->id) == true) {
                me->KaTetNear++;
            }

            PERSON_NEAR)[else]/
            me->KaTetNear--;
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaTet::SM::global::has_katet::near} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state near");
            #endif /* def DESKTOP */
            DISPATCH_END(KATET_NEAR);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaTet::SM::global::has_katet::near::END(PERSON_NEAR)} */
        case END(PERSON_NEAR)_SIG: {
            /*.${SMs::KaTet::SM::global::has_katet::near::END(PERSON_NEAR)::[me->KaTetNear<=1]} */
            if (me->KaTetNear <= 1) {
                me->KaTetNear = 0;
                status_ = Q_TRAN(&KaTet_faraway);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&KaTet_has_katet);
            break;
        }
    }
    return status_;
}
/*.${SMs::KaTet::SM::global::has_katet::faraway} ...........................*/
QState KaTet_faraway(KaTet * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::KaTet::SM::global::has_katet::faraway} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state faraway");
            #endif /* def DESKTOP */

            BEGIN(PERSON_NEAR)[else]/
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaTet::SM::global::has_katet::faraway} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state faraway");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaTet::SM::global::has_katet::faraway::BEGIN(PERSON_NEAR)} */
        case BEGIN(PERSON_NEAR)_SIG: {
            /*.${SMs::KaTet::SM::global::has_katet::faraway::BEGIN(PERSON_NEA~::[(me->KaTets)->get(((constKaTetQ~} */
            if ((me->KaTets)->get(((const KaTetQEvt*)e)->id) == true) {
                status_ = Q_TRAN(&KaTet_near);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&KaTet_has_katet);
            break;
        }
    }
    return status_;
}
/*.${SMs::KaTet::SM::global::alone} ........................................*/
QState KaTet_alone(KaTet * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::KaTet::SM::global::alone} */
        case Q_ENTRY_SIG: {
            #ifdef DESKTOP
            printf("Entered state alone");
            #endif /* def DESKTOP */

            BEGIN(FEAR_PLACE)/
                DISPATCH_BEGIN(FEAR);
                ScreenAddToQueue("Fear.bmp");

            END(FEAR_PLACE)/
                DISPATCH_END(FEAR);
            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaTet::SM::global::alone} */
        case Q_EXIT_SIG: {
            #ifdef DESKTOP
            printf("Exited state alone");
            #endif /* def DESKTOP */

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::KaTet::SM::global::alone::BEGIN(FORM_KATET)} */
        case BEGIN(FORM_KATET)_SIG: {
            (me->KaTets)->set(((const KaTetQEvt*)e)->parameter, true);
                ScreenAddBMPToQueue("Ka_tet.bmp");
                Vibro(MEDIUM_VIBRO);
            status_ = Q_TRAN(&KaTet_faraway);
            break;
        }
        default: {
            status_ = Q_SUPER(&KaTet_global);
            break;
        }
    }
    return status_;
}

#ifdef DESKTOP
/*.${SMs::KaTet::SM::final} ................................................*/
QState KaTet_final(KaTet * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::KaTet::SM::final} */
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

/*.$enddef${SMs::KaTet} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
int main(){
    KaTet_ctor();
    KaTet_initial(me);
}

/*tranlated from diagrams code*/
