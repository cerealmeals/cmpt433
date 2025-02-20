#ifndef RE_STATEMACHINE_H_
#define RE_STATEMACHINE_H_

#include <stdbool.h>

// Define a function pointer type for the callback
typedef void (*StateMachineCallback)(int counter);

void RE_StateMachine_init(void);
void RE_StateMachine_cleanup(void);

int RE_StateMachine_getValue(void);

// TODO: This should be on a background thread (internal?)
int RE_StateMachine_doState(StateMachineCallback callback);

#endif