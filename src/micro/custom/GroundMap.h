#ifndef GROUNDMAP_H
#define GROUNDMAP_H
#include "../core/Graphics.h"
#include "../util/Types.h"
#include "../util/hashmap.h"
#include "../util/vector.h"

typedef struct
{
  unsigned char r0, g0, b0, tmp0;
  unsigned char r1, g1, b1, tmp1;
  unsigned char r2, g2, b2, tmp2;
  unsigned char r3, g3, b3, tmp3;
} GroundTileColors;

typedef struct
{
  int o0, o1, o2, o3;
} GroundTileOrientations;

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
void GroundMapSetTileColors(int tilemapId, int x, int y, unsigned char r0,
                            unsigned char g0, unsigned char b0,
                            unsigned char r1, unsigned char g1,
                            unsigned char b1, unsigned char r2,
                            unsigned char g2, unsigned char b2,
                            unsigned char r3, unsigned char g3,
                            unsigned char b3);
void GroundMapSetTilesColors(int tilemapId, const GroundTileColors *tilesColors,
                             int tile_start_idx, int tiles_count);
void GroundMapSetTileOrientations(int tilemapId, int x, int y, int o0, int o1,
                                  int o2, int o3);
void GroundMapSetTilesOrientations(int tilemapId,
                                   const GroundTileOrientations
                                     *tilesOrientations,
                                   int tile_start_idx, int tiles_count);
void GroundMapApplyChanges(int tilemapId);
void GroundMapFree(int tilemapId);
void GroundMapFreeAll();
void GroundMapsUpdate(float dt);

#endif // TILEMAP_H
