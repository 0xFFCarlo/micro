#ifndef GROUNDMAP_H
#define GROUNDMAP_H
#include "../core/Graphics.h"
#include "../util/Types.h"
#include "../util/hashmap.h"
#include "../util/vector.h"

/////////////////////////////////////////////////////////
/// Tilemap
///
/// Used to render tilemap.
/////////////////////////////////////////////////////////
int GroundMapNew(int textureId, float tx, float ty, float tw, int tileTexSize,
                 int tileSize, int x, int y, int width, int height,
                 int drawLayer, bool visible);
void GroundMapSetVisible(int tilemapId, bool isVisible);
bool GroundMapIsvisible(int tilemapId);
void GroundMapSetPosition(int tilemapId, int x, int y);
void GoundMapGetPosition(int tilemapId, int *x, int *y);
void GroundMapSetColors(int tilemapId, int x, int y, unsigned char r0,
                        unsigned char g0, unsigned char b0, unsigned char r1,
                        unsigned char g1, unsigned char b1, unsigned char r2,
                        unsigned char g2, unsigned char b2, unsigned char r3,
                        unsigned char g3, unsigned char b3);
void GroundMapSetOrientations(int tilemapId, int x, int y, int o0, int o1,
                              int o2, int o3);
void GroundMapApplyChanges(int tilemapId);
void GroundMapFree(int tilemapId);
void GroundMapFreeAll();
void GroundMapsUpdate(float dt);

#endif // TILEMAP_H
