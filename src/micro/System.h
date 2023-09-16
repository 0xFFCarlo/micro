#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

extern void microSystemGetMousePos(int *x, int *y);
extern void microSystemGetWindowSize(int *width, int *height);
extern void microSystemShowCursor(uint8_t show);

#endif /* end of include guard: SYSTEM_H */
