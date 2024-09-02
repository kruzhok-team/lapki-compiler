/*.$file${.::sketch.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: sketch.qm
* File:  ${.::sketch.cpp}
*
*/
/*.$endhead${.::sketch.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qhsm.h"
#include "sketch.h"
#include <stdint.h>
//Q_DEFINE_THIS_FILE
/* global-scope definitions -----------------------------------------*/
QHsm * const the_sketch = (QHsm *) &sketch; /* the opaque pointer *
/*.$define${SMs::Sketch_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Sketch_ctor} .....................................................*/
void Sketch_ctor(void) {
    Sketch *me = &sketch;
     
    QHsm_ctor(&me->super, Q_STATE_CAST(&DEFAULT_Sketch_initial));
}
/*.$enddef${SMs::Sketch_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${SMs::Sketch} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${SMs::Sketch} ..........................................................*/
/*.${SMs::Sketch::SM} ......................................................*/
QState DEFAULT_Sketch_initial(Sketch * const me, void const * const par) {
    /*.${SMs::Sketch::SM::initial} */

    return Q_TRAN(&Sketch_init);
}
/*.${SMs::Sketch::SM::global} ..............................................*/
QState Sketch_global(Sketch * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {

#ifdef DESKTOP
        /*.${SMs::Sketch::SM::global::TERMINATE} */
        case TERMINATE_SIG: {
            status_ = Q_TRAN(&Sketch_final);
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
/*.${SMs::Sketch::SM::global::Включен} .....................................*/
QState Sketch_diod1(Sketch * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Sketch::SM::global::Включен} */
        case Q_ENTRY_SIG: {
            stateChanged = false;
            inVertex = false;
            status_ = Q_HANDLED();

            LED1.on();
            timer1.start(1000);
            break;
        }
        case Q_VERTEX_SIG:
        case QEP_EMPTY_SIG_: {
            status_ = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Sketch::SM::global::Включен::timer1_timeout} */
        case timer1_timeout_SIG: {
            /*.${SMs::Sketch::SM::global::Включен::timer1_timeout::[true]} */
            if (true) {
                stateChanged = true;
                status_ = Q_TRAN(&Sketch_diod2);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Sketch_global);
            break;
        }
    }
    return status_;
}
/*.${SMs::Sketch::SM::global::Выключен} ....................................*/
QState Sketch_diod2(Sketch * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Sketch::SM::global::Выключен} */
        case Q_ENTRY_SIG: {
            stateChanged = false;
            inVertex = false;
            status_ = Q_HANDLED();

            LED1.off();
            timer1.start(1000);
            break;
        }
        case Q_VERTEX_SIG:
        case QEP_EMPTY_SIG_: {
            status_ = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {

            status_ = Q_HANDLED();
            break;
        }
        /*.${SMs::Sketch::SM::global::Выключен::timer1_timeout} */
        case timer1_timeout_SIG: {
            /*.${SMs::Sketch::SM::global::Выключен::timer1_timeout::[true]} */
            if (true) {
                stateChanged = true;
                status_ = Q_TRAN(&Sketch_diod1);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Sketch_global);
            break;
        }
    }
    return status_;
}
/*Initial pseudostate init                                                  */
QState Sketch_init(Sketch * const me, QEvt const * const e) {
         QState status_;
         switch(e -> sig) {
            case Q_ENTRY_SIG: {

              status_ = Q_HANDLED();
              inVertex = true;
              break;
          }
            case Q_EXIT_SIG: {
              inVertex = false;
              status_ = Q_HANDLED();
              break;
          }
          case Q_VERTEX_SIG: {
status_ = Q_TRAN(&Sketch_diod1);
              
              inVertex = false;
              break;
          }
          default: {
              status_ = Q_SUPER(&Sketch_global);
              break;
          }
         }

         return status_;
}


#ifdef DESKTOP
/*.${SMs::Sketch::SM::final} .........................................*/
QState Sketch_final(Sketch * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${SMs::Sketch::SM::final} */
        case Q_ENTRY_SIG: {
            printf("Bye! Bye!"); exit(0);
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

/*.$enddef${SMs::Sketch} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/*tranlated from diagrams code*/

void setup() {
	
	Sketch_ctor();
	QEvt event;
	QMsm_init(the_sketch, &event);
}
void loop() {
    if (inVertex) {
        SIGNAL_DISPATCH(the_sketch, Q_VERTEX_SIG);
    }

  if (stateChanged) {
      signalDefer = true;
      for (int defer_j = 0; defer_j != defer_i; defer_j++) {
          SIGNAL_DISPATCH(the_sketch, defer[defer_j]);
      }
      signalDefer = false;
      stateChanged = false;
      defer_i = 0;
  }
	
	if(timer1.timeout()) {
	    SIMPLE_DISPATCH(the_sketch, timer1_timeout);
	}
	
}