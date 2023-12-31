#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "bsp.h"
#include "service.h"
#include "eventHandlers.h"
#include "qhsm.h"
#include "ability.h"

/*..........................................................................*/

int main() {
    uint8_t     i;                                      // Universal counter
    uint32_t    seconds=0;
    printf("Ability State Machines"\n);
    #ifdef DEBUG_SM
        printf("DEBUG_SM enabled\n\r");
    #endif
    #ifdef DESKTOP
        printf("Desktop version\n\r");
    #endif
    for (i = 0; i < KeyNumber()-1; i++)      // Exluding ESC
        printf("%18s - '%c'\n\r", KeyStrokes[i].Alias, KeyStrokes[i].Key);
    printf("Press ESC to quit...\n");

      /* instantiate the HSM and trigger the initial transition */

    Ability_ctor()

	QMSM_INIT(the_ability, (QEvt *)0);
    QEvt e;
    for (;;) {
        static int tickCtr = 1;
        std::string msg;
        usleep(100000);

        if (kbhit()) {
            char c = _getch();     /* read one character from the console */
            printf("%c: ", c);
            for (i = 0; i < KeyNumber()-1; i++) {
                if (c ==    KeyStrokes[i].Key) {
                    e.sig = KeyStrokes[i].Com;
                    msg = c;
                    break;
                }
            }
        }
        else if (--tickCtr == 0) { /* time for the tick? */
            tickCtr = 10;
            e.sig = TIME_TICK_1S_SIG;
            msg = "TICK";
            seconds++;
            printf("Time: %ds. ", seconds);
        }
		if (msg.length()) {
            /* dispatch the event into the state machine */
            QState r;
            abilityQEvt e
            r = QMSM_DISPATCH(the_ability,  &e);
        }
        #ifdef DEBUG
            printf("returned: %u\n\r", r);
        #endif
   
        }
    }
    return 0;
}
/*..........................................................................*/
void Q_onAssert(char const * const file, int line) {
    fprintf(stderr, "Assertion failed in %s, line %d", file, line);
    _exit(-1);
}
