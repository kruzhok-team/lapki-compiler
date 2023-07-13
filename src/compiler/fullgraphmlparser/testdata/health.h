/*.$file${.::health.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: health.qm
* File:  ${.::health.h}
*
*/
/*.$endhead${.::health.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifndef health_h
#define health_h
#ifdef __cplusplus
extern "C" {
#endif
#include "qhsm.h"    /* include own framework tagunil version */

//Start of h code from diagram
#define SIMPLE 0
#define GOD_READY 1
#define GOD 2
#define DEAD 3
#define DEFAULT_HP 100
#define GOD_THRESHOLD_S 30
#define GOD_PAUSE_M 30
//End of h code from diagram


/*.$declare${SMs::Health} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Health} ..........................................................*/
typedef struct {
/* protected: */
    QHsm super;

/* public: */
    unsigned int health;
    QStateHandler StartState;
    unsigned int count;
    unsigned int god_pause;
} Health;

/* protected: */
QState Health_initial(Health * const me, void const * const par);
QState Health_global(Health * const me, QEvt const * const e);
QState Health_alive(Health * const me, QEvt const * const e);
QState Health_god(Health * const me, QEvt const * const e);
QState Health_mortal(Health * const me, QEvt const * const e);
QState Health_god_ready(Health * const me, QEvt const * const e);
QState Health_simple(Health * const me, QEvt const * const e);
QState Health_dead(Health * const me, QEvt const * const e);

#ifdef DESKTOP
QState Health_final(Health * const me, QEvt const * const e);
#endif /* def DESKTOP */

/*.$enddecl${SMs::Health} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

static Health health; /* the only instance of the Health class */



typedef struct healthQEvt {
    QEvt super;
        int damage;
} healthQEvt;

enum PlayerSignals {
TICK_SEC_SIG = Q_USER_SIG,

MONSTER_SIGNAl_SIG,
PILL_HEAL_SIG,
TIME_TICK_1S_SIG,
MIDDLE_BUTTON_PRESSED_SIG,
TIME_TICK_1M_SIG,
PILL_GOD_SIG,
RAD_RECEIVED_SIG,
PILL_RESET_SIG,
LONG_PRESS_SIG,

LAST_USER_SIG
};
extern QHsm * const the_health; /* opaque pointer to the health HSM */

/*.$declare${SMs::Health_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Health_ctor} .....................................................*/
void Health_ctor(
    unsigned int health,
    unsigned int State,
    unsigned int god_pause);
/*.$enddecl${SMs::Health_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifdef __cplusplus
}
#endif
#endif /* health_h */