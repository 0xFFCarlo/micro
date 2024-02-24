#include "Tilemap.h"
#include "../util/debug.h"
#include "cjson/cJSON.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

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

Tilemap tilemap_load_json(const char *filepath_json)
{
  Tilemap tm;
  tm.layers = NULL;
  tm.num_layers = 0;

  // Read JSON file
  char *jsondata = read_file(filepath_json);
  if (!jsondata)
  {
    debug_print("Failed to read file: %s\n", filepath_json);
    return tm;
  }

  // Parse JSON file
  cJSON *root = cJSON_Parse(jsondata);
  if (!root)
  {
    debug_print("Failed to parse JSON file: %s\n", filepath_json);
    free(jsondata);
    return tm;
  }

  // Read tilemap properties
  cJSON *width = cJSON_GetObjectItem(root, "width");
  cJSON *height = cJSON_GetObjectItem(root, "height");
  cJSON *layers = cJSON_GetObjectItem(root, "layers");
  cJSON *tilewidth = cJSON_GetObjectItem(root, "tilewidth");
  cJSON *tileheight = cJSON_GetObjectItem(root, "tileheight");
  if (!width || !height || !layers || !tilewidth || !tileheight)
  {
    debug_print("Invalid JSON file: %s\n", filepath_json);
    cJSON_Delete(root);
    free(jsondata);
    return tm;
  }
  tm.width = width->valueint;
  tm.height = height->valueint;
  tm.tile_width = tilewidth->valueint;
  tm.tile_height = tileheight->valueint;
  tm.num_layers = cJSON_GetArraySize(layers);
  tm.layers = (TilemapLayer *)malloc(tm.num_layers * sizeof(TilemapLayer));
  if (!tm.layers)
  {
    debug_print("Failed to allocate memory for layers\n");
    cJSON_Delete(root);
    free(jsondata);
    return tm;
  }

  // Load each layer
  // TODO: either data or objects
  for (unsigned int i = 0; i < tm.num_layers; i++)
  {
    cJSON *layer = cJSON_GetArrayItem(layers, i);
    cJSON *name = cJSON_GetObjectItem(layer, "name");
    if (!name)
    {
      debug_print("Invalid layer in JSON file: %s\n", filepath_json);
      cJSON_Delete(root);
      free(jsondata);
      tilemap_free(&tm);
      return tm;
    }

    cJSON *data = cJSON_GetObjectItem(layer, "data");
    cJSON *objects = cJSON_GetObjectItem(layer, "objects");

    if (!data && !objects)
    {
      debug_print("Invalid layer in JSON file: %s\n", filepath_json);
      cJSON_Delete(root);
      free(jsondata);
      tilemap_free(&tm);
      return tm;
    }
    else if (data && objects)
    {
      debug_print("Invalid layer in JSON file: %s\n", filepath_json);
      cJSON_Delete(root);
      free(jsondata);
      tilemap_free(&tm);
      return tm;
    }

    tm.layers[i].name = strdup(name->valuestring);

    if (data) // Tile layer
    {
      tm.layers[i].type = TILE_LAYER;
      tm.layers[i].tile_layer.tiles = (unsigned int *)
        malloc(tm.width * tm.height * sizeof(unsigned int));
      if (!tm.layers[i].tile_layer.tiles)
      {
        debug_print("Failed to allocate memory for layer: %s\n",
                    name->valuestring);
        cJSON_Delete(root);
        free(jsondata);
        tilemap_free(&tm);
        return tm;
      }

      unsigned int tiles_count = cJSON_GetArraySize(data);
      assert(tiles_count == tm.width * tm.height);
      for (unsigned int j = 0; j < tm.width * tm.height; j++)
      {
        cJSON *tile = cJSON_GetArrayItem(data, j);
        tm.layers[i].tile_layer.tiles[j] = tile->valueint;
      }
    }
    else if (objects) // Object layer
    {
      tm.layers[i].type = OBJECT_LAYER;
      unsigned int objects_count = cJSON_GetArraySize(objects);
      tm.layers[i].object_layer.objects_count = objects_count;
      tm.layers[i].object_layer.objects = (Object *)malloc(objects_count *
                                                           sizeof(Object));
      if (!tm.layers[i].object_layer.objects)
      {
        debug_print("Failed to allocate memory for layer: %s\n",
                    name->valuestring);
        cJSON_Delete(root);
        free(jsondata);
        tilemap_free(&tm);
        return tm;
      }

      for (unsigned int j = 0; j < objects_count; j++)
      {
        cJSON *object = cJSON_GetArrayItem(objects, j);
        cJSON *x = cJSON_GetObjectItem(object, "x");
        cJSON *y = cJSON_GetObjectItem(object, "y");
        cJSON *objname = cJSON_GetObjectItem(object, "name");
        if (!x || !y || !objname)
        {
          debug_print("Invalid object in layer: %s\n", name->valuestring);
          cJSON_Delete(root);
          free(jsondata);
          tilemap_free(&tm);
          return tm;
        }

        tm.layers[i].object_layer.objects[j].x = x->valueint;
        tm.layers[i].object_layer.objects[j].y = y->valueint;
        tm.layers[i].object_layer.objects[j].name = strdup(objname
                                                             ->valuestring);
      }
    }
  }

  return tm;
}

int tilemap_get_layer_id(Tilemap *tm, const char *name)
{
  for (unsigned int i = 0; i < tm->num_layers; i++)
    if (strcmp(tm->layers[i].name, name) == 0)
      return i;
  return -1;
}

unsigned int tilemap_get_tile_id(Tilemap *tm, unsigned int layer_id, int x,
                                 int y)
{
  assert(layer_id < tm->num_layers);
  assert(tm->layers[layer_id].type == TILE_LAYER);
  assert(x < (int)tm->width);
  assert(y < (int)tm->height);
  return tm->layers[layer_id].tile_layer.tiles[y * tm->width + x];
}

void tilemap_free(Tilemap *tm)
{
  // Free layers
  if (tm->layers)
  {
    for (unsigned int i = 0; i < tm->num_layers; i++)
    {
      if (tm->layers[i].name)
        free(tm->layers[i].name);

      if (tm->layers[i].type == TILE_LAYER)
      {
        free(tm->layers[i].tile_layer.tiles);
      }
      else if (tm->layers[i].type == OBJECT_LAYER)
      {
        for (unsigned int j = 0; j < tm->layers[i].object_layer.objects_count;
             j++)
          free(tm->layers[i].object_layer.objects[j].name);
        free(tm->layers[i].object_layer.objects);
      }
    }
    free(tm->layers);
  }
}
