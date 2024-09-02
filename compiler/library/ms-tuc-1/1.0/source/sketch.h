/*.$file${.::sketch.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: sketch.qm
* File:  ${.::sketch.h}
*
*/
/*.$endhead${.::sketch.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#pragma once
#ifndef sketch_h
#define sketch_h
#ifdef __cplusplus
extern "C" {
#endif
#include "qhsm.h"    /* include own framework tagunil version */

QSignal defer[10];
int defer_i = 0;
bool stateChanged = false;
bool signalDefer = false;
bool inVertex = false;//Start of h code from diagram
#include "Timer.h"
#include "LED.h"
//End of h code from diagram


/*.$declare${SMs::Sketch} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Sketch} ..........................................................*/
typedef struct {
/* protected: */
    QHsm super;

/* public: */

} Sketch;

/* protected: */
QState DEFAULT_Sketch_initial(Sketch * const me, void const * const par);
QState Sketch_global(Sketch * const me, QEvt const * const e);
QState Sketch_diod1(Sketch * const me, QEvt const * const e);
QState Sketch_diod2(Sketch * const me, QEvt const * const e);
QState Sketch_init(Sketch * const me, QEvt const *const e);

#ifdef DESKTOP
QState Sketch_final(Sketch * const me, QEvt const * const e);
#endif /* def DESKTOP */

/*.$enddecl${SMs::Sketch} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
extern QHsm * const the_sketch; /* opaque pointer to the sketch HSM */

typedef struct sketchQEvt {
    QEvt super;

} sketchQEvt;

enum PlayerSignals {
TICK_SEC_SIG = Q_USER_SIG,

timer1_timeout_SIG,


LAST_USER_SIG
};

static Sketch sketch; /* the only instance of the Sketch class */



/*.$declare${SMs::Sketch_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Sketch_ctor} .....................................................*/
void Sketch_ctor(void);
/*.$enddecl${SMs::Sketch_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
//Start of h code from diagram
LED LED1 = LED(3);

Timer timer1 = Timer();

//End of h code from diagram


#ifdef __cplusplus
}
#endif
#endif /* sketch_h */