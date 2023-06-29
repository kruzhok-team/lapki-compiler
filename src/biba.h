/*.$file${.::biba.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: biba.qm
* File:  ${.::biba.h}
*
*/
/*.$endhead${.::biba.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifndef biba_h
#define biba_h
#ifdef __cplusplus
extern "C" {
#endif
#include "qhsm.h"    /* include own framework tagunil version */

//Start of h code from diagram
#include "Button.h"
#include "Led.h"
//End of h code from diagram


/*.$declare${SMs::Biba} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Biba} ............................................................*/
typedef struct {
/* protected: */
    QHsm super;

/* public: */

} Biba;

/* protected: */
QState Biba_initial(Biba * const me, void const * const par);
QState Biba_global(Biba * const me, QEvt const * const e);
QState Biba_On(Biba * const me, QEvt const * const e);
QState Biba_Off(Biba * const me, QEvt const * const e);

#ifdef DESKTOP
QState Biba_final(Biba * const me, QEvt const * const e);
#endif /* def DESKTOP */

/*.$enddecl${SMs::Biba} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

static Biba biba; /* the only instance of the Biba class */



typedef struct bibaQEvt {
    QEvt super;

} bibaQEvt;

enum PlayerSignals {
TICK_SEC_SIG = Q_USER_SIG,

trig0_SIG,
trig1_SIG,

LAST_USER_SIG
};
extern QHsm * const the_biba; /* opaque pointer to the biba HSM */

/*.$declare${SMs::Biba_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Biba_ctor} .......................................................*/
void Biba_ctor(void);
/*.$enddecl${SMs::Biba_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifdef __cplusplus
}
#endif
#endif /* biba_h */