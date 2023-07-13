/*.$file${.::kaCounter.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: kaCounter.qm
* File:  ${.::kaCounter.h}
*
*/
/*.$endhead${.::kaCounter.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifndef kaCounter_h
#define kaCounter_h
#ifdef __cplusplus
extern "C" {
#endif
#include "qhsm.h"    /* include own framework tagunil version */


/*.$declare${SMs::KaCounter} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::KaCounter} .......................................................*/
typedef struct {
/* protected: */
    QHsm super;
} KaCounter;

/* protected: */
QState KaCounter_initial(KaCounter * const me, void const * const par);
QState KaCounter_global(KaCounter * const me, QEvt const * const e);
QState KaCounter_ka_tet_counter(KaCounter * const me, QEvt const * const e);
QState KaCounter_idle(KaCounter * const me, QEvt const * const e);
QState KaCounter_ka_tet_forming(KaCounter * const me, QEvt const * const e);

#ifdef DESKTOP
QState KaCounter_final(KaCounter * const me, QEvt const * const e);
#endif /* def DESKTOP */

/*.$enddecl${SMs::KaCounter} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

static KaCounter kaCounter; /* the only instance of the KaCounter class */



typedef struct kaCounterQEvt {
    QEvt super;
} kaCounterQEvt;

enum PlayerSignals {
TICK_SEC_SIG = Q_USER_SIG,

TIME_TICK_1M_SIG,
TIME_TICK_1S_SIG,
BEGIN(PERSON_NEAR)_SIG,
END(PERSON_NEAR)_SIG,

LAST_USER_SIG
};
extern QHsm * const the_kaCounter; /* opaque pointer to the kaCounter HSM */

/*.$declare${SMs::KaCounter_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::KaCounter_ctor} ..................................................*/
void KaCounter_ctor(void);
/*.$enddecl${SMs::KaCounter_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifdef __cplusplus
}
#endif
#endif /* kaCounter_h */