#ifndef STATE_HPP
#define STATE_HPP

typedef struct
{
  void (*init)();
  void (*update)(float dt);
  void (*free)();
  double time;
} MicroState;

void microStateSet(MicroState state);
void microStateUpdate(float dt);
void microStateFree();
double microStateGetLastBusyTime();
double microStateGetTime();

#endif // STATE_HPP
