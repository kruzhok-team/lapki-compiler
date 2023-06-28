/*.$file${.::test.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: test.qm
* File:  ${.::test.h}
*
*/
/*.$endhead${.::test.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifndef test_h
#define test_h
#ifdef __cplusplus
extern "C" {
#endif
#include "qhsm.h"    /* include own framework tagunil version */

//Start of h code from diagram
#include "Led.h"
//End of h code from diagram


/*.$declare${SMs::Test} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Test} ............................................................*/
typedef struct {
/* protected: */
    QHsm super;

/* public: */

} Test;

/* protected: */
QState Test_initial(Test * const me, void const * const par);
QState Test_init(Test * const me, QEvt const * const e);
QState Test_Off(Test * const me, QEvt const * const e);
QState Test_On(Test * const me, QEvt const * const e);

#ifdef DESKTOP
QState Test_final(Test * const me, QEvt const * const e);
#endif /* def DESKTOP */

/*.$enddecl${SMs::Test} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

static Test test; /* the only instance of the Test class */



typedef struct testQEvt {
    QEvt super;

} testQEvt;

enum PlayerSignals {
TICK_SEC_SIG = Q_USER_SIG,

Trig1_SIG,
Trig2_SIG,

LAST_USER_SIG
};
extern QHsm * const the_test; /* opaque pointer to the test HSM */

/*.$declare${SMs::Test_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Test_ctor} .......................................................*/
void Test_ctor(void);
/*.$enddecl${SMs::Test_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifdef __cplusplus
}
#endif
#endif /* test_h */