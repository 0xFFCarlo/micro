#ifndef MICRO_H
#define MICRO_H

#include "State.h"

int microInit(MicroState bootState);
int microUpdate(int max_fps);
void microQuit();

#endif // MICRO_H
