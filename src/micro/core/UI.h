#ifndef MICRO_UI_H
#define MICRO_UI_H

#include "Graphics.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum MicroUIContainerType
{
  MICRO_UI_CONT_TYPE_NONE,
  MICRO_UI_CONT_TYPE_TEXT,
  MICRO_UI_CONT_TYPE_SPRITE,
} MicroUIContainerType;

typedef enum MicroUIUnit
{
  MICRO_UI_UNIT_PIXELS,
  MICRO_UI_UNIT_PERCENT,
  MICRO_UI_UNIT_AUTO
} MicroUIUnit;

typedef enum MicroUILayout
{
  MICRO_UI_LAYOUT_HORIZONTAL,
  MICRO_UI_LAYOUT_VERTICAL
} MicroUILayout;

typedef struct MicroUISprite
{
  int textureId;
  MicroTextureRegion tsrc;
  unsigned char color[4];
  unsigned char selectedColor[4];
} MicroUISprite;

typedef struct MicroUIText
{
  unsigned int fontId;
  char *text;
  unsigned int fontSize;
  unsigned int alignment;
  unsigned char color[4];
  unsigned char selectedColor[4];
} MicroUIText;

typedef struct MicroUIContainerDesc
{
  int x, y;
  int width;
  int height;
  int margin[4];  // left, top, right, bottom
  int padding[4]; // left, top, right, bottom
  MicroUILayout layout;

  MicroUIContainerType type;
  union {
    MicroUISprite sprite;
    MicroUIText text;
  };
  bool canSelect;
} MicroUIContainerDesc;

int microUIContainerNew(MicroUIContainerDesc desc);
void microUIContaineChildNew(int parentId, int childId);
void microUIContainerChildRemove(int parentId, int childId);
int microUIContainerGetChildCount(int containerId);
int microUIContainerGetChild(int containerId, int childIndex);
void microUIContainerSet(int containerId, MicroUIContainerDesc desc);
MicroUIContainerDesc microUIContainerGet(int containerId);
MicroUIContainerDesc *microUIContainerModify(int containerId);
void microUIContainerFree(int containerId);
void microUIUpdate();

#endif // MICRO_UI_H
