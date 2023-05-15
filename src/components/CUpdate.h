#ifndef CUPDATE_H
#define CUPDATE_H

typedef struct
{
  void (*update)(int, float);
} CUpdate;

extern int cid_update;
extern void RegisterCUpdate();

#endif /* end of include guard: CUPDATE_H */
