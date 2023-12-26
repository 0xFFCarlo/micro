#ifndef TEST_STATE_H
#define TEST_STATE_H

#include "../micro/State.h"

extern MicroState testStateGet();
extern void testStateInit();
extern void testStateUpdate(float dt);
extern void testStateFree();

#endif /* end of include guard: TEST_STATE_H */
