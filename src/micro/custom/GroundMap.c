#include "GroundMap.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../core/ECS.h"
#include "../util/debug.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define MAX_TILEMAPS 16
#define TM_RENDER_BOUNDS_MARGIN 32
#define CHUNK_HASH_SIZE 2048

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct GroundTilemap
{
  int entityId;
  int textureId;
  int tileSize;
  int x, y;
  int width, height;

  int32_t *bufTileColors;
  uint32_t *bufTileOrientations;
  int startChangeBufColors;
  int endChangeBufColors;
  int startChangeOrientations;
  int endChangeOrientations;
} GroundTilemap;

static GroundTilemap tilemaps[MAX_TILEMAPS];

static MicroAttributeData tile_shader_attrs[] = {
  {0, true, "vpos", 2, MICRO_FLOAT, 0, 0, NULL, false},
  {0, true, "tilemapTransform", 4, MICRO_INT, 0, 1, NULL, false},
  {0, true, "textureInfo", 4, MICRO_INT, 0, 1, NULL, false},
  {0, true, "tileSize", 1, MICRO_INT, 0, 1, NULL, false},
  {0, true, "tileOrientations", 4, MICRO_UNSIGNED_INT, 0, 1, NULL, false},
  {0, true, "tileColors", 4, MICRO_UNSIGNED_INT, 0, 1, NULL, false},
};

static const float quad_verts[2 * 6] = {0,   0,   1.0, 0,   0,   1.0,
                                        1.0, 0.0, 1.0, 1.0, 0.0, 1.0};
static int tilemap_shader_id = -1;

static const char
  *tilemap_shader_frag = "#version 330 core\n"
                         "in vec2 TexCoord;\n"
                         "flat in ivec4 TileOrientations;\n"
                         "flat in vec3 TileColor0;\n"
                         "flat in vec3 TileColor1;\n"
                         "flat in vec3 TileColor2;\n"
                         "flat in vec3 TileColor3;\n"
                         "flat in ivec4 TextureInfo;\n"
                         "out vec4 FragColor;\n"
                         "uniform sampler2D u_texture;\n"
                         "void main() {\n"
                         "  vec2 texSize = vec2(textureSize(u_texture, 0));\n"
                         "  vec2 tileSize = float(TextureInfo.w) / texSize;\n"
                         "  vec2 pixelOrigin = vec2(TextureInfo.x, "
                         "TextureInfo.y) / texSize + TexCoord * tileSize;\n"
                         "\n"
                         "  // accumulate all 4 tile layers into 'color'\n"
                         "  vec4 color = vec4(0.0);\n"
                         "  int tileIdx;\n"
                         "  vec2 texCoord;\n"
                         "\n"
                         "  tileIdx = TileOrientations[0] % TextureInfo.z;\n"
                         "  texCoord = pixelOrigin + vec2(tileIdx, 0) * "
                         "tileSize;\n"
                         "  color += texture(u_texture, texCoord) * "
                         "vec4(TileColor0, 1.0);\n"
                         "\n"
                         "  tileIdx = TileOrientations[1] % TextureInfo.z;\n"
                         "  texCoord = pixelOrigin + vec2(tileIdx, 0) * "
                         "tileSize;\n"
                         "  color += texture(u_texture, texCoord) * "
                         "vec4(TileColor1, 1.0);\n"
                         "\n"
                         "  tileIdx = TileOrientations[2] % TextureInfo.z;\n"
                         "  texCoord = pixelOrigin + vec2(tileIdx, 0) * "
                         "tileSize;\n"
                         "  color += texture(u_texture, texCoord) * "
                         "vec4(TileColor2, 1.0);\n"
                         "\n"
                         "  tileIdx = TileOrientations[3] % TextureInfo.z;\n"
                         "  texCoord = pixelOrigin + vec2(tileIdx, 0) * "
                         "tileSize;\n"
                         "  color += texture(u_texture, texCoord) * "
                         "vec4(TileColor3, 1.0);\n"
                         "\n"
                         "  // branchless fallback: sign(dot)==0 only if "
                         "color.rgb is exactly (0,0,0)\n"
                         "  float fallback = 1.0 - sign(dot(color.rgb, "
                         "color.rgb));\n"
                         "  // when fallback==1 → pick pure red; when 0 → keep "
                         "'color'\n"
                         "  color = mix(\n"
                         "      color,\n"
                         "      vec4(5.0/255.0, 46.0/255.0, 50.0/255.0, 1.0),\n"
                         "      fallback\n"
                         "  );\n"
                         "\n"
                         "  FragColor = color;\n"
                         "}\n";

static const char
  *tilemap_shader_vert = "#version 330 core\n"
                         "layout(location = 0) in vec2 vpos;\n"
                         "layout(location = 1) in ivec4 tilemapTransform;\n"
                         "layout(location = 2) in ivec4 textureInfo;\n"
                         "layout(location = 3) in int tileSize;\n"
                         "layout(location = 4) in ivec4 tileOrientations;\n"
                         "layout(location = 5) in ivec4 tileColors;\n"
                         "out vec2 TexCoord;\n"
                         "flat out ivec4 TextureInfo;\n"
                         "flat out ivec4 TileOrientations;\n"
                         "flat out vec3 TileColor0;\n"
                         "flat out vec3 TileColor1;\n"
                         "flat out vec3 TileColor2;\n"
                         "flat out vec3 TileColor3;\n"
                         "uniform mat4 u_view;\n"
                         "\n"
                         "void main() {\n"
                         "  vec2 tilePos = vec2(gl_InstanceID % "
                         "tilemapTransform.z,\n"
                         "                      gl_InstanceID / "
                         "tilemapTransform.z) *\n"
                         "                 tileSize;\n"
                         "  vec2 worldPos = vpos * tileSize + tilePos + "
                         "tilemapTransform.xy;\n"
                         "    worldPos = floor(worldPos);\n"
                         "  gl_Position = u_view * vec4(worldPos, 0.0, 1.0);\n"
                         "  TexCoord = vpos;\n"
                         "  TextureInfo = textureInfo;\n"
                         "  TileOrientations = tileOrientations;\n"
                         "  TileColor0 = vec3(tileColors[0] & 0xFF, "
                         "(tileColors[0] >> 8) & 0xFF,\n"
                         "                    (tileColors[0] >> 16) & 0xFF) / "
                         "255.0;\n"
                         "  TileColor1 = vec3(tileColors[1] & 0xFF, "
                         "(tileColors[1] >> 8) & 0xFF,\n"
                         "                    (tileColors[1] >> 16) & 0xFF) / "
                         "255.0;\n"
                         "  TileColor2 = vec3(tileColors[2] & 0xFF, "
                         "(tileColors[2] >> 8) & 0xFF,\n"
                         "                    (tileColors[2] >> 16) & 0xFF) / "
                         "255.0;\n"
                         "  TileColor3 = vec3(tileColors[3] & 0xFF, "
                         "(tileColors[3] >> 8) & 0xFF,\n"
                         "                    (tileColors[3] >> 16) & 0xFF) / "
                         "255.0;\n"
                         "}";

int GroundMapNew(int textureId, float tx, float ty, float tw, int tileTexSize,
                 int tileSize, int x, int y, int width, int height,
                 int drawLayer, bool visible)
{
  assert(tileSize > 0);
  assert(width > 0);
  assert(height > 0);

  // Make sure tilemap shader is compiled
  if (tilemap_shader_id == -1)
  {
    tilemap_shader_id = microShaderLoadFromSource("_ground_tilemap_shader",
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

  GroundTilemap *tm = &tilemaps[spot];
  tm->textureId = textureId;
  tm->tileSize = tileSize;
  tm->x = x;
  tm->y = y;
  tm->width = width;
  tm->height = height;
  tm->bufTileColors = malloc(sizeof(uint32_t) * width * height * 4);
  tm->bufTileOrientations = malloc(sizeof(uint32_t) * width * height * 4);
  tm->startChangeBufColors = 0;
  tm->endChangeBufColors = 0;
  tm->startChangeOrientations = 0;
  tm->endChangeOrientations = 0;

  tm->entityId = microECSEntityNew(NULL, NULL);
  CmpAddPosition(tm->entityId, tm->x, tm->y);
  CmpAddDrawable(tm->entityId, drawLayer, visible);
  tile_shader_attrs[0].vbo_id = microVBONew(2 * 6 * sizeof(float),
                                            MICRO_STATIC_DRAW, quad_verts);
  tile_shader_attrs[1].vbo_id = microVBONew(4 * sizeof(int), MICRO_STATIC_DRAW,
                                            NULL);
  // Use same tilemapTransform for all tiles
  tile_shader_attrs[1].divisor = width * height;
  tile_shader_attrs[2].vbo_id = microVBONew(4 * sizeof(int), MICRO_STATIC_DRAW,
                                            NULL);
  tile_shader_attrs[2].divisor = width * height;
  tile_shader_attrs[3].vbo_id = microVBONew(1 * sizeof(int), MICRO_STATIC_DRAW,
                                            NULL);
  tile_shader_attrs[3].divisor = width * height;
  tile_shader_attrs[4].vbo_id = microVBONew(4 * sizeof(u32) * tm->width *
                                              tm->height,
                                            MICRO_STATIC_DRAW, NULL);
  tile_shader_attrs[5].vbo_id = microVBONew(4 * sizeof(u32) * tm->width *
                                              tm->height,
                                            MICRO_STATIC_DRAW, NULL);
  CmpAddMesh(tm->entityId, tilemap_shader_id, textureId, 6,
             tm->width * tm->height, tile_shader_attrs,
             sizeof(tile_shader_attrs) / sizeof(tile_shader_attrs[0]));
  CMesh *mesh = CmpGetMesh(tm->entityId);
  microVAOSubmit(mesh->VAOId, "vpos", quad_verts, 0, 6);
  int tilemapTransform[4] = {tm->x, tm->y, tm->width, tm->height};
  microVAOSubmit(mesh->VAOId, "tilemapTransform", tilemapTransform, 0, 1);
  debug_print("Created ground tilemap at %d, %d\n", tm->x, tm->y);
  int texWidth, texHeight;
  microTextureGetSize(textureId, &texWidth, &texHeight);
  int textureInfo[4] = {tx, ty, tw / tileTexSize, tileTexSize};
  microVAOSubmit(mesh->VAOId, "textureInfo", textureInfo, 0, 1);
  microVAOSubmit(mesh->VAOId, "tileSize", &tileSize, 0, 1);
  microVAOSetDrawRange(mesh->VAOId, 0, 6, tm->width * tm->height, 0);
  return spot;
}

void GroundMapSetVisible(int tilemapId, bool isVisible)
{
  GroundTilemap *tm = &tilemaps[tilemapId];
  CDrawable *draw = CmpGetDrawable(tm->entityId);
  draw->visible = isVisible;
}

bool GroundMapIsvisible(int tilemapId)
{
  GroundTilemap *tm = &tilemaps[tilemapId];
  CDrawable *draw = CmpGetDrawable(tm->entityId);
  return draw->visible;
}

void GroundMapSetPosition(int tilemapId, int x, int y)
{
  GroundTilemap *tm = &tilemaps[tilemapId];
  CPosition *pos = CmpGetPosition(tm->entityId);
  pos->x = x;
  pos->y = y;
  CMesh *mesh = CmpGetMesh(tm->entityId);
  int tilemapTransform[4] = {x, y, tm->width, tm->height};
  microVAOSubmit(mesh->VAOId, "tilemapTransform", tilemapTransform, 0, 1);
}

void GroundMapGetPosition(int tilemapId, int *x, int *y)
{
  GroundTilemap *tm = &tilemaps[tilemapId];
  CPosition *pos = CmpGetPosition(tm->entityId);
  *x = pos->x;
  *y = pos->y;
}

void GroundMapSetTileColors(int tilemapId, int x, int y, unsigned char r0,
                            unsigned char g0, unsigned char b0,
                            unsigned char r1, unsigned char g1,
                            unsigned char b1, unsigned char r2,
                            unsigned char g2, unsigned char b2,
                            unsigned char r3, unsigned char g3,
                            unsigned char b3)
{
  GroundTilemap *tm = &tilemaps[tilemapId];
  tm->bufTileColors[(x + y * tm->width) * 4 + 0] = r0 | (g0 << 8) | (b0 << 16);
  tm->bufTileColors[(x + y * tm->width) * 4 + 1] = r1 | (g1 << 8) | (b1 << 16);
  tm->bufTileColors[(x + y * tm->width) * 4 + 2] = r2 | (g2 << 8) | (b2 << 16);
  tm->bufTileColors[(x + y * tm->width) * 4 + 3] = r3 | (g3 << 8) | (b3 << 16);
  tm->startChangeBufColors = MIN(tm->startChangeBufColors, x + y * tm->width);
  tm->endChangeBufColors = MAX(tm->endChangeBufColors, x + y * tm->width + 1);
}

void GroundMapSetTilesColors(int tilemapId, const GroundTileColors *tilesColors,
                             int tile_start_idx, int tiles_count)
{
  GroundTilemap *tm = &tilemaps[tilemapId];
  assert(tile_start_idx + tiles_count <= tm->width * tm->height);
  memcpy(&tm->bufTileColors[tile_start_idx * sizeof(GroundTileColors)],
         tilesColors, sizeof(GroundTileColors) * tiles_count);
  tm->startChangeBufColors = MIN(tm->startChangeBufColors, tile_start_idx);
  tm->endChangeBufColors = MAX(tm->endChangeBufColors,
                               tile_start_idx + tiles_count);
}

void GroundMapSetTileOrientations(int tilemapId, int x, int y, int o0, int o1,
                                  int o2, int o3)
{
  GroundTilemap *tm = &tilemaps[tilemapId];
  tm->bufTileOrientations[(x + y * tm->width) * 4 + 0] = o0;
  tm->bufTileOrientations[(x + y * tm->width) * 4 + 1] = o1;
  tm->bufTileOrientations[(x + y * tm->width) * 4 + 2] = o2;
  tm->bufTileOrientations[(x + y * tm->width) * 4 + 3] = o3;
  tm->startChangeOrientations = MIN(tm->startChangeOrientations,
                                    x + y * tm->width);
  tm->endChangeOrientations = MAX(tm->endChangeOrientations,
                                  x + y * tm->width + 1);
}

void GroundMapSetTilesOrientations(int tilemapId,
                                   const GroundTileOrientations
                                     *tilesOrientations,
                                   int tile_start_idx, int tiles_count)
{
  GroundTilemap *tm = &tilemaps[tilemapId];
  assert(tile_start_idx + tiles_count <= tm->width * tm->height);
  memcpy(&tm->bufTileOrientations[tile_start_idx * sizeof(GroundTileColors)],
         tilesOrientations, sizeof(GroundTileOrientations) * tiles_count);
  tm->startChangeOrientations = MIN(tm->startChangeOrientations,
                                    tile_start_idx);
  tm->endChangeOrientations = MAX(tm->endChangeOrientations,
                                  tile_start_idx + tiles_count);
}

void GroundMapApplyChanges(int tilemapId)
{
  GroundTilemap *tm = &tilemaps[tilemapId];
  CMesh *mesh = CmpGetMesh(tm->entityId);

  // Apply tile id changes, if any
  if (tm->startChangeBufColors != tm->width * tm->height * 4)
  {
    microVAOSubmit(mesh->VAOId, "tileColors",
                   &tm->bufTileColors[tm->startChangeBufColors],
                   tm->startChangeBufColors,
                   tm->endChangeBufColors - tm->startChangeBufColors);
    tm->startChangeBufColors = tm->width * tm->height;
    tm->endChangeBufColors = 0;
  }

  if (tm->startChangeOrientations != tm->width * tm->height * 4)
  {
    microVAOSubmit(mesh->VAOId, "tileOrientations",
                   &tm->bufTileOrientations[tm->startChangeOrientations],
                   tm->startChangeOrientations,
                   tm->endChangeOrientations - tm->startChangeOrientations);
    tm->startChangeOrientations = tm->width * tm->height;
    tm->endChangeOrientations = 0;
  }
}

void GroundMapFree(int tilemapId)
{
  assert(tilemapId >= 0 && tilemapId < MAX_TILEMAPS);
  GroundTilemap *tm = &tilemaps[tilemapId];
  tm->width = 0;
  free(tm->bufTileColors);
  free(tm->bufTileOrientations);
  microECSEntityQueueFree(tm->entityId);
}

void GroundMapFreeAll()
{
  for (int i = 0; i < MAX_TILEMAPS; i++)
    if (tilemaps[i].width != 0)
      GroundMapFree(i);
}

void GroundMapsUpdate(float dt)
{
  (void)dt;

  if (tilemap_shader_id == -1)
    return;

  microShaderApply(tilemap_shader_id);
  microViewApply();
}
