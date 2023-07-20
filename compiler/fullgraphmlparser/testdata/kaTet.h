/*.$file${.::kaTet.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: kaTet.qm
* File:  ${.::kaTet.h}
*
*/
/*.$endhead${.::kaTet.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifndef kaTet_h
#define kaTet_h
#ifdef __cplusplus
extern "C" {
#endif
#include "qhsm.h"    /* include own framework tagunil version */


/*.$declare${SMs::KaTet} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::KaTet} ...........................................................*/
typedef struct {
/* protected: */
    QHsm super;
} KaTet;

/* protected: */
QState KaTet_initial(KaTet * const me, void const * const par);
QState KaTet_global(KaTet * const me, QEvt const * const e);
QState KaTet_has_katet(KaTet * const me, QEvt const * const e);
QState KaTet_near(KaTet * const me, QEvt const * const e);
QState KaTet_faraway(KaTet * const me, QEvt const * const e);
QState KaTet_alone(KaTet * const me, QEvt const * const e);

#ifdef DESKTOP
QState KaTet_final(KaTet * const me, QEvt const * const e);
#endif /* def DESKTOP */

/*.$enddecl${SMs::KaTet} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

static KaTet kaTet; /* the only instance of the KaTet class */



typedef struct kaTetQEvt {
    QEvt super;
} kaTetQEvt;

enum PlayerSignals {
TICK_SEC_SIG = Q_USER_SIG,

BEGIN(FORM_KATET)_SIG,
BEGIN(DESTROY_KATET)_SIG,
BEGIN(PERSON_NEAR)_SIG,
END(PERSON_NEAR)_SIG,

LAST_USER_SIG
};
extern QHsm * const the_kaTet; /* opaque pointer to the kaTet HSM */

/*.$declare${SMs::KaTet_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::KaTet_ctor} ......................................................*/
void KaTet_ctor(void);
/*.$enddecl${SMs::KaTet_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifdef __cplusplus
}
#endif
#endif /* kaTet_h */