#ifndef CLOCK_ON_VIEW_H
#define CLOCK_ON_VIEW_H

typedef struct {
  unsigned char followRotation;
} CLockOnView;

extern int cid_lock_on_view;
extern void RegisterCLockOnView();

#endif /* end of include guard: CLOCK_ON_VIEW_H */
