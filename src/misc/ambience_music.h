#ifndef AMBIENCE_MUSIC_H
#define AMBIENCE_MUSIC_H

#include <stdint.h>

typedef enum AmbienceType
{
  AMBIENCE_NONE,
  AMBIENCE_NORMAL,
  AMBIENCE_INVASION,
  AMBIENCE_COUNT
} AmbienceType;

extern void ambienceMusicSetup();
extern void ambienceMusicSet(AmbienceType type);

#endif
