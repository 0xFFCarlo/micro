#ifndef TILEMAP_H
#define TILEMAP_H
#include "../util/Types.h"
#include "../util/hashmap.h"
#include "../util/vector.h"
#include "Graphics.h"

#define MAX_PROPERTY_NAME_LEN 24
#define MAX_CHUNKS_IN_VIEW 16

typedef struct TileLayerDesc
{
  uint32_t *tiles;
  uint32_t tiles_types;
} TileLayerDesc;

typedef enum PropertyType
{
  PROPERTY_TYPE_INT,
  PROPERTY_TYPE_FLOAT,
  PROPERTY_TYPE_STRING
} PropertyType;

typedef struct ObjectProperty
{
  char name[MAX_PROPERTY_NAME_LEN];
  PropertyType type;
  union {
    int32_t int_value;
    float float_value;
    char *string_value;
  };
} ObjectProperty;

typedef struct ObjectDesc
{
  char *name;
  int32_t x;
  int32_t y;
  uint32_t width;
  uint32_t height;
  ObjectProperty *properties;
  uint32_t properties_count;
} ObjectDesc;

typedef struct ObjectLayerDesc
{
  ObjectDesc *objects;
  uint32_t objects_count;
} ObjectLayerDesc;

typedef struct ImageLayerDesc
{
  char *image_name;
  float opacity;
} ImageLayerDesc;

typedef enum LayerType
{
  TILE_LAYER,
  OBJECT_LAYER,
  IMAGE_LAYER
} LayerType;

typedef struct TilemapLayerDesc
{
  char *name;
  LayerType type;
  union {
    TileLayerDesc tile_layer;
    ObjectLayerDesc object_layer;
    ImageLayerDesc image_layer;
  };
} TilemapLayerDesc;

typedef struct TilemapDesc
{
  uint32_t width;
  uint32_t height;
  uint32_t tile_width;
  uint32_t tile_height;
  TilemapLayerDesc *layers;
  uint32_t num_layers;
} TilemapDesc;

typedef struct TilemapChunk
{
  int16_t x; // chunk grid position X (in tiles * tile_width * chunk_width)
  int16_t y; // chunk grid position Y (in tiles * tile_height * chunk_height)
  float active_timer; // Check if chunk is active (>0) or not (=0)
  bool in_view;       // Check if chunk is in view
  struct TilemapChunk *neighbors[3][3]; // Neighbors chunks
  void *data;
} TilemapChunk;

typedef struct TilemapDynamic
{
  uint32_t eid;              // Tilemap entity id
  HashMap chunks;            // Hashmap of chunks
  Vector chunks_active;      // List of active chunks pointers
  uint32_t chunk_width;      // number of tiles in chunk width
  uint32_t chunk_height;     // number of tiles in chunk height
  uint32_t tile_width;       // tile width in pixels
  uint32_t tile_height;      // tile height in pixels
  uint32_t active_radius_px; // radius of active chunks
  float active_timeout_s;    // time to keep chunk active

  TilemapChunk *(*chunk_new)(struct TilemapDynamic *tilemap, int16_t chunk_x,
                             int16_t chunk_y);
  void (*chunk_update)(struct TilemapDynamic *tm, TilemapChunk *chunk,
                       float dt);
} TilemapDynamic;

typedef struct TileAnimation
{
  uint8_t framesCount;
  uint8_t animationSpeed;
  uint8_t animationOffset;
  uint8_t animationStride;
} TileAnimation;

/////////////////////////////////////////////////////////
/// Tilemap Description
///
/// Used to store, load and save tilemap description.
/////////////////////////////////////////////////////////

// Load tilemap description from JSON file.
TilemapDesc *microTilemapDescLoadJson(const char *filepath_json);

// Get layer id by name.
int32_t microTilemapDescGetLayerID(TilemapDesc *tm, const char *name);

// Get tile ID by layer ID and tile position.
uint32_t microTilemapDescGetTile(TilemapDesc *tm, uint32_t layer_id, int32_t x,
                                 int32_t y);

// Free tilemap description.
void microTilemapDescFree(TilemapDesc *tm);

// Get object property by name
ObjectProperty *microObjectDescGetProperty(ObjectDesc *obj, const char *name);

/////////////////////////////////////////////////////////
/// Tilemap
///
/// Used to render tilemap.
/////////////////////////////////////////////////////////

// Create tilemap
TilemapDynamic *microTilemapDynamicNew(uint32_t chunk_width,
                                       uint32_t chunk_height,
                                       uint32_t tile_width,
                                       uint32_t tile_height,
                                       uint32_t active_radius_px,
                                       float active_timeout_s,
                                       TilemapChunk
                                         *(*chunk_new)(TilemapDynamic *tilemap,
                                                       int16_t chunk_x,
                                                       int16_t chunk_y),
                                       void (*chunk_free)(TilemapChunk *chunk),
                                       void (*chunk_update)(TilemapDynamic *tm,
                                                            TilemapChunk *chunk,
                                                            float dt));
void microTilemapAddChunk(TilemapDynamic *tm, TilemapChunk *chunk);
void microTilemapDynamicFree(TilemapDynamic *tm);
TilemapChunk *microTilemapDynamicGetChunkAt(TilemapDynamic *tm, int32_t x,
                                            int32_t y);

/////////////////////////////////////////////////////////
/// Tilemap
///
/// Used to render tilemap.
/////////////////////////////////////////////////////////
typedef struct MicroTileAnimation
{
  uint8_t framesCount;
  uint8_t animationSpeed;
  uint8_t animationOffset;
  uint8_t animationStride;
} MicroTileAnimation;
int microTilemapNew(int textureId, float tx, float ty, float tw,
                    int tileTexSize, int tileSize, int x, int y, int width,
                    int height, int drawLayer, bool visible);
void microTilemapSetVisible(int tilemapId, bool isVisible);
bool microTilemapIsvisible(int tilemapId);
void microTilemapSetPosition(int tilemapId, int x, int y);
void microTilemaoGetPosition(int tilemapId, int *x, int *y);
void microTilemapSetTile(int tilemapId, int x, int y, int tileId);
void microTilemapSetTiles(int tilemapId, int *tile_ids, int tile_start_idx,
                          int tiles_count);
int microTilemapGetTile(int tilemapId, int x, int y);
void microTilemapSetTileAnimation(int tilemapId, int x, int y,
                                  uint8_t framesCount, uint8_t animSpeed,
                                  uint8_t animOffset, uint8_t animStride);
void microTilemapSetTilesAnimation(int tilemapId,
                                   MicroTileAnimation *tile_anim_infos,
                                   int start_idx, int tiles_count);
TileAnimation microTilemapGetTileAnimation(int tilemapId, int x, int y);
void microTilemapApplyChanges(int tilemapId);
void microTilemapFree(int tilemapId);
void microTilemapFreeAll();
void microTilemapsUpdate(float dt);

#endif // TILEMAP_H
