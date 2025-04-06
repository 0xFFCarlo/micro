#include "Tilemap.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../util/debug.h"
#include "ECS.h"
#include "cjson/cJSON.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define MAX_TILEMAPS 16
#define TM_RENDER_BOUNDS_MARGIN 32
#define CHUNK_HASH_SIZE 2048

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

char *read_file(const char *filename)
{
  FILE *file = fopen(filename, "rb");
  if (!file)
  {
    debug_print("Failed to open file: %s\n", filename);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(length + 1);
  if (!buffer)
  {
    debug_print("Failed to allocate memory for file: %s\n", filename);
    fclose(file);
    return NULL;
  }

  fread(buffer, 1, length, file);
  fclose(file);
  buffer[length] = '\0';
  return buffer;
}

TilemapDesc *microTilemapDescLoadJson(const char *filepath_json)
{
  TilemapDesc *tm = NULL;
  cJSON *root = NULL;
  char *jsondata = NULL;
  bool valid = false;

  // Read JSON file
  jsondata = read_file(filepath_json);
  if (!jsondata)
  {
    debug_print("Failed to read file: %s\n", filepath_json);
    return tm;
  }

  // Parse JSON file
  root = cJSON_Parse(jsondata);
  if (!root)
  {
    debug_print("Failed to parse JSON file: %s\n", filepath_json);
    goto exit;
  }

  tm = (TilemapDesc *)malloc(sizeof(TilemapDesc));
  tm->layers = NULL;
  tm->num_layers = 0;

  // Read tilemap properties
  const cJSON *width = cJSON_GetObjectItem(root, "width");
  const cJSON *height = cJSON_GetObjectItem(root, "height");
  cJSON *layers = cJSON_GetObjectItem(root, "layers");
  const cJSON *tilewidth = cJSON_GetObjectItem(root, "tilewidth");
  const cJSON *tileheight = cJSON_GetObjectItem(root, "tileheight");
  if (!width || !height || !layers || !tilewidth || !tileheight)
  {
    debug_print("Invalid JSON file: %s\n", filepath_json);
    goto exit;
  }
  tm->width = width->valueint;
  tm->height = height->valueint;
  tm->tile_width = tilewidth->valueint;
  tm->tile_height = tileheight->valueint;
  tm->num_layers = cJSON_GetArraySize(layers);
  tm->layers = (TilemapLayerDesc *)malloc(tm->num_layers *
                                          sizeof(TilemapLayerDesc));
  if (!tm->layers)
  {
    debug_print("Failed to allocate memory for layers\n");
    goto exit;
  }

  // Load each layer
  for (uint32_t i = 0; i < tm->num_layers; i++)
  {
    cJSON *js_layer = cJSON_GetArrayItem(layers, i);
    const cJSON *name = cJSON_GetObjectItem(js_layer, "name");
    if (!name)
    {
      debug_print("Invalid layer in JSON file: %s\n", filepath_json);
      goto exit;
    }

    tm->layers[i].name = malloc(strlen(name->valuestring) + 1);
    strcpy(tm->layers[i].name, name->valuestring);

    cJSON *data = cJSON_GetObjectItem(js_layer, "data");
    cJSON *js_objects = cJSON_GetObjectItem(js_layer, "objects");
    cJSON *image = cJSON_GetObjectItem(js_layer, "image");

    if (!data && !js_objects && !image)
    {
      debug_print("Invalid layer in JSON file: %s\n", filepath_json);
      goto exit;
    }
    else if (data && js_objects)
    {
      debug_print("Invalid layer in JSON file: %s\n", filepath_json);
      goto exit;
    }

    if (data) // Tile layer
    {
      tm->layers[i].type = TILE_LAYER;
      tm->layers[i].tile_layer.tiles = (uint32_t *)malloc(tm->width *
                                                          tm->height *
                                                          sizeof(uint32_t));
      if (!tm->layers[i].tile_layer.tiles)
      {
        debug_print("Failed to allocate memory for layer: %s\n",
                    name->valuestring);
        goto exit;
      }

      uint32_t tiles_count = cJSON_GetArraySize(data);
      assert(tiles_count == tm->width * tm->height);
      for (uint32_t j = 0; j < tm->width * tm->height; j++)
      {
        cJSON *tile = cJSON_GetArrayItem(data, j);
        tm->layers[i].tile_layer.tiles[j] = tile->valueint;
      }
    }
    else if (js_objects) // Object layer
    {
      tm->layers[i].type = OBJECT_LAYER;
      uint32_t objects_count = cJSON_GetArraySize(js_objects);
      tm->layers[i].object_layer.objects_count = objects_count;
      tm->layers[i].object_layer.objects = (ObjectDesc *)
        malloc(objects_count * sizeof(ObjectDesc));
      ObjectDesc *objects = tm->layers[i].object_layer.objects;

      if (!tm->layers[i].object_layer.objects)
      {
        debug_print("Failed to allocate memory for layer: %s\n",
                    name->valuestring);
        goto exit;
      }

      for (uint32_t j = 0; j < objects_count; j++)
      {
        cJSON *js_object = cJSON_GetArrayItem(js_objects, j);
        cJSON *x = cJSON_GetObjectItem(js_object, "x");
        cJSON *y = cJSON_GetObjectItem(js_object, "y");
        cJSON *width = cJSON_GetObjectItem(js_object, "width");
        cJSON *height = cJSON_GetObjectItem(js_object, "height");
        cJSON *objname = cJSON_GetObjectItem(js_object, "name");
        if (!x || !y || !objname)
        {
          debug_print("Invalid object in layer: %s\n", name->valuestring);
          goto exit;
        }

        // Store object data
        objects[j].x = x->valueint;
        objects[j].y = y->valueint;
        objects[j].width = width ? width->valueint : 0;
        objects[j].height = height ? height->valueint : 0;
        objects[j].name = malloc(strlen(objname->valuestring) + 1);
        strcpy(objects[j].name, objname->valuestring);
        objects[j].properties = NULL;
        objects[j].properties_count = 0;

        // Load custom properties
        cJSON *properties = cJSON_GetObjectItem(js_object, "properties");
        if (properties)
        {
          const uint32_t properties_count = cJSON_GetArraySize(properties);
          objects[j].properties = (ObjectProperty *)
            malloc(properties_count * sizeof(ObjectProperty));
          objects[j].properties_count = properties_count;
          for (uint32_t k = 0; k < properties_count; k++)
          {
            cJSON *prop = cJSON_GetArrayItem(properties, k);
            cJSON *prop_name = cJSON_GetObjectItem(prop, "name");
            cJSON *prop_type = cJSON_GetObjectItem(prop, "type");
            cJSON *prop_value = cJSON_GetObjectItem(prop, "value");
            if (!prop_name || !prop_type || !prop_value)
            {
              debug_print("Invalid property in object: %s\n",
                          objname->valuestring);
              goto exit;
            }

            // Store property data
            ObjectProperty *property = &objects[j].properties[k];
            strncpy(property->name, prop_name->valuestring,
                    MAX_PROPERTY_NAME_LEN);
            const char *ptype_str = cJSON_GetStringValue(prop_type);
            if (strcmp(ptype_str, "int") == 0)
            {
              property->type = PROPERTY_TYPE_INT;
              property->int_value = prop_value->valueint;
            }
            else if (strcmp(ptype_str, "float") == 0)
            {
              property->type = PROPERTY_TYPE_FLOAT;
              property->float_value = prop_value->valuedouble;
            }
            else if (strcmp(ptype_str, "string") == 0)
            {
              property->type = PROPERTY_TYPE_STRING;
              property->string_value = malloc(strlen(prop_value->valuestring) +
                                              1);
              strcpy(property->string_value, prop_value->valuestring);
            }
            else
            {
              debug_print("Invalid property type in object: %s\n",
                          objname->valuestring);
              goto exit;
            }
          }
        }
      }
    }
    else if (image)
    {
      tm->layers[i].type = IMAGE_LAYER;
      tm->layers[i].image_layer.image_name = malloc(strlen(image->valuestring) +
                                                    1);
      strcpy(tm->layers[i].image_layer.image_name, image->valuestring);
      cJSON *opacity = cJSON_GetObjectItem(js_layer, "opacity");
      if (opacity)
      {
        tm->layers[i].image_layer.opacity = cJSON_GetNumberValue(opacity);
      }
    }
  }

  valid = true;

exit:

  if (tm && !valid)
    microTilemapDescFree(tm);

  if (root)
    cJSON_Delete(root);

  if (jsondata)
    free(jsondata);

  return tm;
}

int32_t microTilemapDescGetLayerID(TilemapDesc *tm, const char *name)
{
  for (uint32_t i = 0; i < tm->num_layers; i++)
    if (strcmp(tm->layers[i].name, name) == 0)
      return i;
  return -1;
}

uint32_t microTilemapDescGetTile(TilemapDesc *tm, uint32_t layer_id, int32_t x,
                                 int32_t y)
{
  assert(layer_id < tm->num_layers);
  assert(tm->layers[layer_id].type == TILE_LAYER);
  assert(x < (int32_t)tm->width);
  assert(y < (int32_t)tm->height);
  return tm->layers[layer_id].tile_layer.tiles[y * tm->width + x];
}

void microTilemapDescFree(TilemapDesc *tm)
{
  // Free layers
  if (tm->layers && tm->num_layers > 0)
  {
    for (uint32_t i = 0; i < tm->num_layers; i++)
    {
      if (tm->layers[i].name)
        free(tm->layers[i].name);

      if (tm->layers[i].type == TILE_LAYER)
      {
        if (tm->layers[i].tile_layer.tiles)
          free(tm->layers[i].tile_layer.tiles);
      }
      else if (tm->layers[i].type == OBJECT_LAYER)
      {

        // Clear objects
        if (tm->layers[i].object_layer.objects)
        {
          ObjectLayerDesc *obj_layer = &tm->layers[i].object_layer;

          // Clear object names and properties
          for (uint32_t j = 0; j < obj_layer->objects_count; j++)
          {
            // Clear name
            if (obj_layer->objects[j].name)
              free(obj_layer->objects[j].name);

            // Clearf properties
            for (uint32_t k = 0; k < obj_layer->objects[j].properties_count;
                 k++)
            {
              ObjectProperty *prop = &obj_layer->objects[j].properties[k];

              if (prop->type == PROPERTY_TYPE_STRING && prop->string_value)
                free(prop->string_value);
            }
            if (obj_layer->objects[j].properties)
              free(tm->layers[i].object_layer.objects[j].properties);
          }

          free(tm->layers[i].object_layer.objects);
        }
      }
      else if (tm->layers[i].type == IMAGE_LAYER)
      {
        if (tm->layers[i].image_layer.image_name)
          free(tm->layers[i].image_layer.image_name);
      }
    }
    free(tm->layers);
  }
  free(tm);
}

ObjectProperty *microObjectDescGetProperty(ObjectDesc *obj, const char *name)
{
  for (uint32_t i = 0; i < obj->properties_count; i++)
    if (strcmp(obj->properties[i].name, name) == 0)
      return &obj->properties[i];
  return NULL;
}

uint32_t chunk_hash(const void *key)
{
  const uint16_t *p = (const uint16_t *)key;

  uint32_t k = (uint32_t)p[0] * 73856093 + (uint32_t)p[1] * 19349663;

  // Fold to 11 bits
  k ^= k >> 16;
  k = (k ^ (k >> 8)) & (CHUNK_HASH_SIZE - 1);
  return k;
}

static float clamp(float value, float min, float max)
{
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

static bool is_chunk_in_range(float cx, float cy, float radius, float view_left,
                              float view_right, float view_top,
                              float view_bottom)
{
  // Find the closest point to the circle within the square
  float closestX = clamp(cx, view_left, view_right);
  float closestY = clamp(cy, view_top, view_bottom);

  // Calculate the distance between the circle's center and this closest point
  float dx = closestX - cx;
  float dy = closestY - cy;

  // Calculate the squared distance and compare with squared radius
  float distanceSquared = dx * dx + dy * dy;

  // If the distance is less than or equal to the radius squared, they overlap
  return distanceSquared <= radius * radius;
}

void microTilemapChunkUpdateNeighbours(TilemapDynamic *tm, TilemapChunk *chunk)
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      int16_t key[2] = {chunk->x + i - 1, chunk->y + j - 1};
      TilemapChunk *n = hashmap_get(&tm->chunks, key);
      chunk->neighbors[i][j] = n;
      if (n)
        n->neighbors[2 - i][2 - j] = chunk;
    }
  }
}

void microTilemapAddChunk(TilemapDynamic *tm, TilemapChunk *chunk)
{
  int16_t key[2] = {chunk->x, chunk->y};
  hashmap_insert(&tm->chunks, key, chunk);
  vector_push_back(&tm->chunks_active, &chunk);
  microTilemapChunkUpdateNeighbours(tm, chunk);
}

static void microTilemapDynamicUpdate(int eid, float dt)
{
  TilemapDynamic *tm = microECSEntityGetData(eid);

  // Get view bounds
  float viewCX, viewCY;
  float viewWidth, viewHeight;
  microViewGetCenter(&viewCX, &viewCY);
  microViewGetSize(&viewWidth, &viewHeight);

  // Calculate rendering bounds in pixel coordinates
  const int32_t view_left = viewCX - viewWidth / 2.f - TM_RENDER_BOUNDS_MARGIN;
  const int32_t view_right = viewCX + viewWidth / 2.f + TM_RENDER_BOUNDS_MARGIN;
  const int32_t view_top = viewCY - viewHeight / 2.f - TM_RENDER_BOUNDS_MARGIN;
  const int32_t view_bottom = viewCY + viewHeight / 2.f +
                              TM_RENDER_BOUNDS_MARGIN;

  // Chunk size in px
  const int32_t chunk_width_px = tm->chunk_width * tm->tile_width;
  const int32_t chunk_height_px = tm->chunk_height * tm->tile_height;

  // Make sure chunks exists
  // NOTE: make a circle instead of a square
  for (int x = viewCX - tm->active_radius_px;
       x <= viewCX + tm->active_radius_px; x += chunk_width_px)
  {
    for (int y = viewCY - tm->active_radius_px;
         y <= viewCY + tm->active_radius_px; y += chunk_height_px)
    {
      const int16_t key[2] = {
        (float)x / (float)(tm->chunk_width * tm->tile_width),
        (float)y / (float)(tm->chunk_height * tm->tile_height)};

      // Make sure the chunk is in the active radius
      if (is_chunk_in_range(viewCX, viewCY, tm->active_radius_px,
                            key[0] * chunk_width_px,
                            (key[0] + 1) * chunk_width_px,
                            key[1] * chunk_height_px,
                            (key[1] + 1) * chunk_height_px) == false)
        continue;

      TilemapChunk *chunk = hashmap_get(&tm->chunks, key);
      if (chunk == NULL)
      {
        chunk = tm->chunk_new(tm, key[0], key[1]);
        chunk->in_view = false;
        chunk->active_timer = tm->active_timeout_s; // make it active
        microTilemapAddChunk(tm, chunk);
      }
      else if (chunk->active_timer <= 0)
      {
        chunk->active_timer = tm->active_timeout_s;
        vector_push_back(&tm->chunks_active, &chunk);
      }
    }
  }

  // Update active chunks and remove inactive ones
  for (int i = tm->chunks_active.size - 1; i >= 0; i--)
  {
    TilemapChunk *chunk = *(TilemapChunk **)vector_at(&tm->chunks_active, i);

    if (chunk == NULL)
      continue;

    // Update if the chunk is in view or not
    chunk->in_view = false;
    if (chunk->x * chunk_width_px < view_right &&
        (chunk->x + 1) * chunk_width_px > view_left &&
        chunk->y * chunk_height_px < view_bottom &&
        (chunk->y + 1) * chunk_height_px > view_top)
    {
      chunk->in_view = true;
    }

    if (is_chunk_in_range(viewCX, viewCY, tm->active_radius_px,
                          chunk->x * chunk_width_px,
                          (chunk->x + 1) * chunk_width_px,
                          chunk->y * chunk_height_px,
                          (chunk->y + 1) * chunk_height_px))
      chunk->active_timer = tm->active_timeout_s;
    else
      chunk->active_timer -= dt;

    // Remove chunk if it's inactive
    if (chunk->active_timer <= 0)
    {
      debug_print("Removing chunk: %d, %d\n", chunk->x, chunk->y);
      vector_remove(&tm->chunks_active, i);
      continue;
    }

    // Update chunk
    tm->chunk_update(tm, chunk, dt);
  }
}

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
                                                            float dt))
{
  TilemapDynamic *tm = malloc(sizeof(TilemapDynamic));
  tm->chunks = hashmap_create(sizeof(int16_t) * 2, CHUNK_HASH_SIZE, chunk_hash,
                              (void (*)(void *))chunk_free);
  tm->chunks_active = vector_create(sizeof(TilemapChunk *));
  tm->chunk_width = chunk_width;
  tm->chunk_height = chunk_height;
  tm->tile_width = tile_width;
  tm->tile_height = tile_height;
  tm->active_radius_px = active_radius_px;
  tm->active_timeout_s = active_timeout_s;
  tm->chunk_new = chunk_new;
  tm->chunk_update = chunk_update;
  tm->eid = microECSEntityNew(tm, NULL);
  CmpAddPosition(tm->eid, 0, 0);
  CmpAddUpdate(tm->eid, microTilemapDynamicUpdate);
  return tm;
}

void microTilemapDynamicFree(TilemapDynamic *tm)
{
  microECSEntityRemove(tm->eid);
  hashmap_free(&tm->chunks);
  vector_free(&tm->chunks_active);
  free(tm);
}

TilemapChunk *microTilemapDynamicGetChunkAt(TilemapDynamic *tm, int32_t x,
                                            int32_t y)
{
  const int16_t key[2] = {(int16_t)x, (int16_t)y};
  TilemapChunk *chunk = hashmap_get(&tm->chunks, key);
  return chunk;
}

typedef struct MicroTilemap
{
  int entityId;
  int textureId;
  int tileSize;
  int x, y;
  int width, height;

  float *bufInstancePos;
  int32_t *bufTileId;
  uint32_t *bufAnimInfo;
  int startChangeBufTileId;
  int endChangeBufTileId;
  int startChangeAnimInfo;
  int endChangeAnimInfo;
} MicroTilemap;

MicroTilemap tilemaps[MAX_TILEMAPS];

static MicroAttributeData tile_shader_attrs[] = {
  {0, true, "vpos", 2, MICRO_FLOAT, 0, 0, NULL, false},
  {0, true, "tilemapTransform", 4, MICRO_INT, 0, 1, NULL, false},
  {0, true, "textureInfo", 2, MICRO_INT, 0, 1, NULL, false},
  {0, true, "tileSize", 1, MICRO_INT, 0, 1, NULL, false},
  {0, true, "tileId", 1, MICRO_INT, 0, 1, NULL, false},
  {0, true, "animationInfo", 1, MICRO_UNSIGNED_INT, 0, 1, NULL, false},
};

static const float quad_verts[2 * 6] = {0,   0,   1.0, 0,   0,   1.0,
                                        1.0, 0.0, 1.0, 1.0, 0.0, 1.0};
static int tilemap_shader_id = -1;
static double tilemap_shader_time = 0.0;

static const char
  *tilemap_shader_frag = "#version 330 core\n"
                         "in vec2 TexCoord;\n"
                         "flat in int TileID;\n"
                         "flat in ivec2 TextureInfo;\n"
                         "out vec4 FragColor;\n"
                         "uniform sampler2D u_texture;\n"
                         "void main() {\n"
                         "    int tileCol = TileID % TextureInfo.x;\n"
                         "    int tileRow = TileID / TextureInfo.x;\n"
                         "    vec2 tileSize = 1.0 / vec2(TextureInfo);\n"
                         "    vec2 tileBaseCoord = vec2(tileCol, tileRow) * "
                         "tileSize;\n"
                         "    vec2 finalTexCoord = tileBaseCoord + TexCoord * "
                         "tileSize;\n"
                         "    FragColor = texture(u_texture, finalTexCoord);\n"
                         "}";

static const char *
  tilemap_shader_vert = "#version 330 core\n"
                        "layout(location = 0) in vec2 vpos;\n"
                        "layout(location = 1) in ivec4 tilemapTransform;\n"
                        "layout(location = 2) in ivec2 textureInfo;\n"
                        "layout(location = 3) in int tileSize;\n"
                        "layout(location = 4) in int tileId;\n"
                        "layout(location = 5) in int animationInfo;\n"
                        "out vec2 TexCoord;\n"
                        "flat out ivec2 TextureInfo;\n"
                        "flat out int TileID;\n"
                        "uniform mat4 u_view;\n"
                        "uniform float time;\n"
                        "void main() {\n"
                        "    if (tileId == -1) {\n"
                        "        gl_Position = vec4(0.0, 0.0, 0.0, 0.0);\n"
                        "        return;\n"
                        "    }\n"
                        "    vec2 tilePos = vec2(gl_InstanceID % "
                        "tilemapTransform.z, gl_InstanceID / "
                        "tilemapTransform.z) * tileSize;\n"
                        "    vec2 worldPos = vpos * tileSize + tilePos + "
                        "tilemapTransform.xy;\n"
                        "    gl_Position = u_view * vec4(worldPos, 0.0, 1.0);\n"
                        "    int animFrames = animationInfo & 0xFF;\n"
                        "    float animSpeed = float((animationInfo >> 8) & "
                        "0xFF) / 4.0;\n"
                        "    float animOffset = float((animationInfo >> 16) & "
                        "0xFF) / 4.0;\n"
                        "    int animFrameStride = (animationInfo >> 24) & "
                        "0xFF;\n"
                        "    TileID = tileId + (int(time * animSpeed + "
                        "animOffset) % animFrames) * animFrameStride;\n"
                        "    TexCoord = vpos;\n"
                        "    TextureInfo = textureInfo;\n"
                        "}";

int microTilemapNew(int textureId, int tileTexSize, int tileSize, int x, int y,
                    int width, int height, int drawLayer, bool visible)
{
  assert(tileSize > 0);
  assert(width > 0);
  assert(height > 0);

  // Make sure tilemap shader is compiled
  if (tilemap_shader_id == -1)
  {
    tilemap_shader_id = microShaderLoadFromSource("_tilemap_shader",
                                                  tilemap_shader_vert,
                                                  tilemap_shader_frag);
    if (tilemap_shader_id == -1)
    {
      debug_print("Failed to load tilemap shader\n");
      abort_trace();
    }
  }

  int spot = -1;
  for (int i = 0; i < MAX_TILEMAPS; i++)
  {
    if (tilemaps[i].width != 0)
      continue;
    spot = i;
    break;
  }

  if (spot == -1)
  {
    debug_print("Error: could not allocate tilemap!\n");
    abort_trace();
  }

  MicroTilemap *tm = &tilemaps[spot];
  tm->textureId = textureId;
  tm->tileSize = tileSize;
  tm->x = x;
  tm->y = y;
  tm->width = width;
  tm->height = height;
  tm->bufInstancePos = malloc(sizeof(float) * width * height * 2);
  tm->bufTileId = malloc(sizeof(int32_t) * width * height);
  tm->bufAnimInfo = malloc(sizeof(uint32_t) * width * height);
  tm->startChangeBufTileId = 0;
  tm->endChangeBufTileId = 0;
  tm->startChangeAnimInfo = 0;
  tm->endChangeAnimInfo = 0;

  tm->entityId = microECSEntityNew(NULL, NULL);
  CmpAddPosition(tm->entityId, tm->x, tm->y);
  CmpAddDrawable(tm->entityId, drawLayer, visible);
  tile_shader_attrs[0].vbo_id = microVBONew(2 * 6 * sizeof(float),
                                            MICRO_STATIC_DRAW, quad_verts);
  tile_shader_attrs[1].vbo_id = microVBONew(4 * sizeof(int), MICRO_STATIC_DRAW,
                                            NULL);
  // Use same tilemapTransform for all tiles
  tile_shader_attrs[1].divisor = width * height;
  tile_shader_attrs[2].vbo_id = microVBONew(2 * sizeof(int), MICRO_STATIC_DRAW,
                                            NULL);
  tile_shader_attrs[2].divisor = width * height;
  tile_shader_attrs[3].vbo_id = microVBONew(1 * sizeof(int), MICRO_STATIC_DRAW,
                                            NULL);
  tile_shader_attrs[3].divisor = width * height;
  tile_shader_attrs[4].vbo_id = microVBONew(sizeof(u32) * tm->width *
                                              tm->height,
                                            MICRO_STATIC_DRAW, NULL);
  tile_shader_attrs[5].vbo_id = microVBONew(sizeof(u32) * tm->width *
                                              tm->height,
                                            MICRO_STATIC_DRAW, NULL);
  CmpAddMesh(tm->entityId, tilemap_shader_id, textureId, 6,
             tm->width * tm->height, tile_shader_attrs,
             sizeof(tile_shader_attrs) / sizeof(tile_shader_attrs[0]));
  CMesh *mesh = CmpGetMesh(tm->entityId);
  microVAOSubmit(mesh->VAOId, "vpos", quad_verts, 0, 6);
  int tilemapTransform[4] = {tm->x, tm->y, tm->width, tm->height};
  microVAOSubmit(mesh->VAOId, "tilemapTransform", tilemapTransform, 0, 1);
  debug_print("Created tilemap at %d, %d\n", tm->x, tm->y);
  int texWidth, texHeight;
  microTextureGetSize(textureId, &texWidth, &texHeight);
  int textureInfo[2] = {texWidth / tileTexSize, texHeight / tileTexSize};
  microVAOSubmit(mesh->VAOId, "textureInfo", textureInfo, 0, 1);
  microVAOSubmit(mesh->VAOId, "tileSize", &tileSize, 0, 1);
  microVAOSetDrawRange(mesh->VAOId, 0, 6, tm->width * tm->height, 0);
  return spot;
}

void microTilemapSetVisible(int tilemapId, bool isVisible)
{
  MicroTilemap *tm = &tilemaps[tilemapId];
  CDrawable *draw = CmpGetDrawable(tm->entityId);
  draw->visible = isVisible;
}

bool microTilemapIsvisible(int tilemapId)
{
  MicroTilemap *tm = &tilemaps[tilemapId];
  CDrawable *draw = CmpGetDrawable(tm->entityId);
  return draw->visible;
}

void microTilemapSetPosition(int tilemapId, int x, int y)
{
  MicroTilemap *tm = &tilemaps[tilemapId];
  CPosition *pos = CmpGetPosition(tm->entityId);
  pos->x = x;
  pos->y = y;
  CMesh *mesh = CmpGetMesh(tm->entityId);
  int tilemapTransform[4] = {x, y, tm->width, tm->height};
  microVAOSubmit(mesh->VAOId, "tilemapTransform", tilemapTransform, 0, 1);
}

void microTilemaoGetPosition(int tilemapId, int *x, int *y)
{
  MicroTilemap *tm = &tilemaps[tilemapId];
  CPosition *pos = CmpGetPosition(tm->entityId);
  *x = pos->x;
  *y = pos->y;
}

void microTilemapSetTile(int tilemapId, int x, int y, int tileId)
{
  MicroTilemap *tm = &tilemaps[tilemapId];
  tm->bufTileId[x + y * tm->width] = tileId;
  tm->startChangeBufTileId = MIN(tm->startChangeBufTileId, x + y * tm->width);
  tm->endChangeBufTileId = MAX(tm->endChangeBufTileId, x + y * tm->width + 1);
}

void microTilemapSetTiles(int tilemapId, int *tile_ids, int tile_start_idx,
                          int tiles_count)
{
  MicroTilemap *tm = &tilemaps[tilemapId];
  assert(tile_start_idx + tiles_count <= tm->width * tm->height);
  memcpy(&tm->bufTileId[tile_start_idx], tile_ids, sizeof(int) * tiles_count);
  tm->startChangeBufTileId = MIN(tm->startChangeBufTileId, tile_start_idx);
  tm->endChangeBufTileId = MAX(tm->endChangeBufTileId,
                               tile_start_idx + tiles_count);
}

int microTilemapGetTile(int tilemapId, int x, int y)
{
  MicroTilemap *tm = &tilemaps[tilemapId];
  return tm->bufTileId[x + y * tm->width];
}

void microTilemapSetTileAnimation(int tilemapId, int x, int y,
                                  uint8_t framesCount, uint8_t animSpeed,
                                  uint8_t animOffset, uint8_t animStride)
{
  MicroTilemap *tm = &tilemaps[tilemapId];
  uint32_t *info = &tm->bufAnimInfo[x + y * tm->width];
  *info = (uint32_t)framesCount | ((uint32_t)animSpeed << 8) |
          ((uint32_t)animOffset << 16) | ((uint32_t)animStride << 24);
  tm->startChangeAnimInfo = MIN(tm->startChangeAnimInfo, x + y * tm->width);
  tm->endChangeAnimInfo = MAX(tm->endChangeAnimInfo, x + y * tm->width + 1);
}

void microTilemapSetTilesAnimation(int tilemapId,
                                   MicroTileAnimation *tile_anim_infos,
                                   int start_idx, int tiles_count)
{
  MicroTilemap *tm = &tilemaps[tilemapId];
  assert(start_idx + tiles_count <= tm->width * tm->height);
  memcpy(&tm->bufAnimInfo[start_idx], tile_anim_infos,
         sizeof(MicroTileAnimation) * tiles_count);
  tm->startChangeAnimInfo = MIN(tm->startChangeAnimInfo, start_idx);
  tm->endChangeAnimInfo = MAX(tm->endChangeAnimInfo, start_idx + tiles_count);
}

TileAnimation microTilemapGetTileAnimation(int tilemapId, int x, int y)
{
  MicroTilemap *tm = &tilemaps[tilemapId];
  uint32_t info = tm->bufAnimInfo[x + y * tm->width];
  TileAnimation animInfo;
  animInfo.framesCount = info & 0xFF;
  animInfo.animationSpeed = (info >> 8) & 0xFF;
  animInfo.animationOffset = (info >> 16) & 0xFF;
  animInfo.animationStride = (info >> 24) & 0xFF;
  return animInfo;
}

void microTilemapApplyChanges(int tilemapId)
{
  MicroTilemap *tm = &tilemaps[tilemapId];
  CMesh *mesh = CmpGetMesh(tm->entityId);

  // Apply tile id changes, if any
  if (tm->startChangeBufTileId != tm->width * tm->height)
  {
    microVAOSubmit(mesh->VAOId, "tileId",
                   &tm->bufTileId[tm->startChangeBufTileId],
                   tm->startChangeBufTileId,
                   tm->endChangeBufTileId - tm->startChangeBufTileId);
    tm->startChangeBufTileId = tm->width * tm->height;
    tm->endChangeBufTileId = 0;
  }

  // Apply tile animation changes, if any
  if (tm->startChangeAnimInfo != tm->width * tm->height)
  {
    microVAOSubmit(mesh->VAOId, "animationInfo",
                   &tm->bufAnimInfo[tm->startChangeAnimInfo],
                   tm->startChangeAnimInfo,
                   tm->endChangeAnimInfo - tm->startChangeAnimInfo);
    tm->startChangeAnimInfo = tm->width * tm->height;
    tm->endChangeAnimInfo = 0;
  }
}

void microTilemapFree(int tilemapId)
{
  assert(tilemapId >= 0 && tilemapId < MAX_TILEMAPS);
  MicroTilemap *tm = &tilemaps[tilemapId];
  tm->width = 0;
  free(tm->bufTileId);
  free(tm->bufInstancePos);
  free(tm->bufAnimInfo);
  microECSEntityRemove(tm->entityId);
}

void microTilemapFreeAll()
{
  for (int i = 0; i < MAX_TILEMAPS; i++)
    if (tilemaps[i].width != 0)
      microTilemapFree(i);
}

void microTilemapsUpdate(float dt)
{
  if (tilemap_shader_id == -1)
    return;

  tilemap_shader_time += dt;
  microShaderApply(tilemap_shader_id);
  microShaderSetUniformf("time", tilemap_shader_time);
  microViewApply();
}
