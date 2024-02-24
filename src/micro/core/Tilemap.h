#ifndef TILEMAP_H
#define TILEMAP_H

typedef struct TileLayer {
  unsigned int *tiles;
  unsigned int tiles_types;
} TileLayer;

typedef struct Object {
  char *name;
  int x;
  int y;
} Object;

typedef struct ObjectLayer {
  Object *objects;
  unsigned int objects_count;
} ObjectLayer;

typedef enum LayerType {
  TILE_LAYER,
  OBJECT_LAYER
} LayerType;

typedef struct TilemapLayer {
  char *name;
  LayerType type;
  union {
    TileLayer tile_layer;
    ObjectLayer object_layer;
  };
} TilemapLayer;

typedef struct Tilemap {
    unsigned int width;
    unsigned int height;
    unsigned int tile_width;
    unsigned int tile_height;
    TilemapLayer *layers;
    unsigned int num_layers;
} Tilemap;

Tilemap tilemap_load_json(const char *filepath_json);
int tilemap_get_layer_id(Tilemap *tm, const char *name);
unsigned int tilemap_get_tile_id(Tilemap *tm, unsigned int layer_id, int x, int y);
void tilemap_free(Tilemap *tm);

#endif // TILEMAP_H
