#ifndef CANIMATION_H
#define CANIMATION_H

typedef struct {
    int animationId;
    int frameId;
    float timeSinceLastFrame;
} CAnimation;

extern int cid_animation;
extern void RegisterCAnimation();

#endif /* end of include guard: CANIMATION_H */
