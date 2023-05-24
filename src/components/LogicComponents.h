#ifndef LOGIC_COMPONENTS_H
#define LOGIC_COMPONENTS_H

// Update
typedef struct
{
  void (*update)(int, float);
} CUpdate;

extern int cid_update;
extern void RegisterCUpdate();

#endif /* end of include guard: LOGIC_COMPONENTS_H */
