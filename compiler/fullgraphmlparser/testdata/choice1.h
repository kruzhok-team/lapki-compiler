/*.$file${.::choice1.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: choice1.qm
* File:  ${.::choice1.h}
*
*/
/*.$endhead${.::choice1.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifndef choice1_h
#define choice1_h
#ifdef __cplusplus
extern "C" {
#endif
#include "qhsm.h"    /* include own framework tagunil version */


/*.$declare${SMs::Choice1} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Choice1} .........................................................*/
typedef struct {
/* protected: */
    QHsm super;
} Choice1;

/* protected: */
QState Choice1_initial(Choice1 * const me, void const * const par);
QState Choice1_global(Choice1 * const me, QEvt const * const e);
QState Choice1_parent(Choice1 * const me, QEvt const * const e);
QState Choice1_idle(Choice1 * const me, QEvt const * const e);
QState Choice1_choice1(Choice1 * const me, QEvt const * const e);
QState Choice1_choice2(Choice1 * const me, QEvt const * const e);

#ifdef DESKTOP
QState Choice1_final(Choice1 * const me, QEvt const * const e);
#endif /* def DESKTOP */

/*.$enddecl${SMs::Choice1} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

static Choice1 choice1; /* the only instance of the Choice1 class */



typedef struct choice1QEvt {
    QEvt super;
} choice1QEvt;

enum PlayerSignals {
TICK_SEC_SIG = Q_USER_SIG,

TRIGGER_SIG,

LAST_USER_SIG
};
extern QHsm * const the_choice1; /* opaque pointer to the choice1 HSM */

/*.$declare${SMs::Choice1_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Choice1_ctor} ....................................................*/
void Choice1_ctor(void);
/*.$enddecl${SMs::Choice1_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifdef __cplusplus
}
#endif
#endif /* choice1_h */