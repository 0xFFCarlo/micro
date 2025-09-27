#ifndef STATE_HPP
#define STATE_HPP

typedef struct
{
  void (*init)();
  void (*update)(float dt);
  void (*free)();
  double time;
  float dt;
} MicroState;

void microStateSet(MicroState state);
void microStateUpdate(float dt);
void microStateFree();
double microStateGetLastBusyTime();
double microStateGetTime();
float microStateGetDeltaTime();

#endif // STATE_HPP
