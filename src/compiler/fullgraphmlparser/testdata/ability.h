/*.$file${.::ability.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: ability.qm
* File:  ${.::ability.h}
*
*/
/*.$endhead${.::ability.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifndef ability_h
#define ability_h
#ifdef __cplusplus
extern "C" {
#endif
#include "qhsm.h"    /* include own framework tagunil version */

//Start of h code from diagram
#define ABILITY_THRESHOLD_S 30
#define ABILITY_PAUSE_M
//End of h code from diagram


/*.$declare${SMs::Ability} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Ability} .........................................................*/
typedef struct {
/* protected: */
    QHsm super;

/* public: */
    unsigned int ability_pause;
    unsigned int count;
    unsigned int ability;
} Ability;

/* protected: */
QState Ability_initial(Ability * const me, void const * const par);
QState Ability_global(Ability * const me, QEvt const * const e);
QState Ability_ability(Ability * const me, QEvt const * const e);
QState Ability_idle(Ability * const me, QEvt const * const e);
QState Ability_active(Ability * const me, QEvt const * const e);

#ifdef DESKTOP
QState Ability_final(Ability * const me, QEvt const * const e);
#endif /* def DESKTOP */

/*.$enddecl${SMs::Ability} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

static Ability ability; /* the only instance of the Ability class */



typedef struct abilityQEvt {
    QEvt super;
} abilityQEvt;

enum PlayerSignals {
TICK_SEC_SIG = Q_USER_SIG,

TIME_TICK_1M_SIG,
PILL_ABILITY_SIG,
LONG_PRESS_THIRD_SIG,
TIME_TICK_1S_SIG,

LAST_USER_SIG
};
extern QHsm * const the_ability; /* opaque pointer to the ability HSM */

/*.$declare${SMs::Ability_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Ability_ctor} ....................................................*/
void Ability_ctor(
    unsigned int ability_pause,
    unsigned int ability);
/*.$enddecl${SMs::Ability_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifdef __cplusplus
}
#endif
#endif /* ability_h */