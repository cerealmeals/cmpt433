#ifndef _BTN_STATEMACHINE_H_
#define _BTN_STATEMACHINE_H_

#include <stdbool.h>

void RE_StateMachine_init(void);
void RE_StateMachine_cleanup(void);

int RE_StateMachine_getValue(void);

// TODO: This should be on a background thread (internal?)
void RE_StateMachine_doState();

#endif