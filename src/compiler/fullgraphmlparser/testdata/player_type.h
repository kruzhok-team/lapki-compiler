/*.$file${.::player_type.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: player_type.qm
* File:  ${.::player_type.h}
*
*/
/*.$endhead${.::player_type.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifndef player_type_h
#define player_type_h
#ifdef __cplusplus
extern "C" {
#endif
#include "qhsm.h"    /* include own framework tagunil version */

//Start of h code from diagram
#define LOCAL 1
#define TAILOR 2
#define STALKER 3
#define DEAD 0
#define REGEN_THRESH_S 60
#define LOCAL_HP 40
#define TAILOR_HP 60
#define STALKER_HP 100
//End of h code from diagram


/*.$declare${SMs::Player_type} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Player_type} .....................................................*/
typedef struct {
/* protected: */
    QHsm super;

/* public: */
    unsigned int TimerRegen;
    Health* CharHealth;
} Player_type;

/* protected: */
QState Player_type_initial(Player_type * const me, void const * const par);
QState Player_type_global(Player_type * const me, QEvt const * const e);
QState Player_type_player_type(Player_type * const me, QEvt const * const e);
QState Player_type_alive(Player_type * const me, QEvt const * const e);
QState Player_type_may_regenerate(Player_type * const me, QEvt const * const e);
QState Player_type_normal(Player_type * const me, QEvt const * const e);
QState Player_type_regenerating(Player_type * const me, QEvt const * const e);
QState Player_type_tailor(Player_type * const me, QEvt const * const e);
QState Player_type_stalker(Player_type * const me, QEvt const * const e);
QState Player_type_dead(Player_type * const me, QEvt const * const e);

#ifdef DESKTOP
QState Player_type_final(Player_type * const me, QEvt const * const e);
#endif /* def DESKTOP */

/*.$enddecl${SMs::Player_type} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

static Player_type player_type; /* the only instance of the Player_type class */



typedef struct player_typeQEvt {
    QEvt super;
    int value;
} player_typeQEvt;

enum PlayerSignals {
TICK_SEC_SIG = Q_USER_SIG,

PILL_RESET_SIG,
DMG_RCVD_SIG,
TIME_TICK_1S_SIG,
PILL_TAILOR_SIG,
PILL_STALKER_SIG,
PILL_LOCAL_SIG,

LAST_USER_SIG
};
extern QHsm * const the_player_type; /* opaque pointer to the player_type HSM */

/*.$declare${SMs::Player_type_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Player_type_ctor} ................................................*/
void Player_type_ctor(
    Health* CharHealth,
    unsigned int TimeRegen,
    unsigned int State);
/*.$enddecl${SMs::Player_type_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#ifdef __cplusplus
}
#endif
#endif /* player_type_h */