#ifndef TEST_STATE_H
#define TEST_STATE_H

#include "../micro/core/State.h"

MicroState testStateGet();
void testStateInit();
void testStateUpdate(float dt);
void testStateFree();

#endif /* end of include guard: TEST_STATE_H */
