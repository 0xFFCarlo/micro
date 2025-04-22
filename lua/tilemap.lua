local ffi = require("ffi")
local lib = require("micro/lua/libmicro")

ffi.cdef([[
typedef struct MicroTileAnimation {
  uint8_t framesCount;
  uint8_t animationSpeed;
  uint8_t animationOffset;
  uint8_t animationStride;
} MicroTileAnimation;

typedef struct TileAnimation {
  uint8_t framesCount;
  uint8_t animationSpeed;
  uint8_t animationOffset;
  uint8_t animationStride;
} TileAnimation;

int microTilemapNew(int textureId, float tx, float ty, float tw, int tileTexSize, int tileSize, int x, int y,
                    int width, int height, int drawLayer, bool visible);

void microTilemapSetVisible(int tilemapId, bool isVisible);
bool microTilemapIsvisible(int tilemapId);
void microTilemapSetPosition(int tilemapId, int x, int y);
void microTilemapGetPosition(int tilemapId, int *x, int *y);

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
]])

local Tilemap = {}

--[[
Creates a new tilemap.
@param textureId number
@param tx number
@param ty number
@param tw number
@param tileTexSize number
@param tileSize number
@param x number
@param y number
@param width number
@param height number
@param drawLayer number
@param visible boolean
@return number tilemapId
]]
Tilemap.new = function(textureId, tx, ty, tw, tileTexSize, tileSize, x, y, width, height, drawLayer, visible)
	return lib.microTilemapNew(textureId, tx, ty, tw, tileTexSize, tileSize, x, y, width, height, drawLayer, visible)
end

--[[
Sets tilemap visibility.
@param tilemapId number
@param isVisible boolean
]]
Tilemap.setVisible = function(tilemapId, isVisible)
	lib.microTilemapSetVisible(tilemapId, isVisible)
end

--[[
Checks if tilemap is visible.
@param tilemapId number
@return boolean
]]
Tilemap.isVisible = function(tilemapId)
	return lib.microTilemapIsvisible(tilemapId)
end

--[[
-- Sets tilemap position.
-- @param tilemapId number
-- @param x number
-- @param y number
-- ]]
Tilemap.setPosition = function(tilemapId, x, y)
	lib.microTilemapSetPosition(tilemapId, x, y)
end

--[[
-- Gets tilemap position.
-- @param tilemapId number
-- @return number x
-- @return number y
-- ]]
Tilemap.getPosition = function(tilemapId)
	local x = ffi.new("int[1]")
	local y = ffi.new("int[1]")
	lib.microTilemapGetPosition(tilemapId, x, y)
	return x[0], y[0]
end

--[[
Sets tile at (x,y).
@param tilemapId number
@param x number
@param y number
@param tileId number
]]
Tilemap.setTile = function(tilemapId, x, y, tileId)
	lib.microTilemapSetTile(tilemapId, x, y, tileId)
end

--[[
Sets multiple tiles.
@param tilemapId number
@param tile_ids ffi pointer to int array
@param tile_start_idx number
@param tiles_count number
]]
Tilemap.setTiles = function(tilemapId, tile_ids, tile_start_idx, tiles_count)
	lib.microTilemapSetTiles(tilemapId, tile_ids, tile_start_idx, tiles_count)
end

--[[
Gets tile at (x,y).
@param tilemapId number
@param x number
@param y number
@return number tileId
]]
Tilemap.getTile = function(tilemapId, x, y)
	return lib.microTilemapGetTile(tilemapId, x, y)
end

--[[
Sets tile animation at (x,y).
@param tilemapId number
@param x number
@param y number
@param framesCount number (uint8)
@param animSpeed number (uint8)
@param animOffset number (uint8)
@param animStride number (uint8)
]]
Tilemap.setTileAnimation = function(tilemapId, x, y, framesCount, animSpeed, animOffset, animStride)
	lib.microTilemapSetTileAnimation(tilemapId, x, y, framesCount, animSpeed, animOffset, animStride)
end

--[[
Sets animations for multiple tiles.
@param tilemapId number
@param tile_anim_infos ffi pointer to MicroTileAnimation array
@param start_idx number
@param tiles_count number
]]
Tilemap.setTilesAnimation = function(tilemapId, tile_anim_infos, start_idx, tiles_count)
	lib.microTilemapSetTilesAnimation(tilemapId, tile_anim_infos, start_idx, tiles_count)
end

--[[
Gets tile animation at (x,y).
@param tilemapId number
@param x number
@param y number
@return TileAnimation
]]
Tilemap.getTileAnimation = function(tilemapId, x, y)
	return lib.microTilemapGetTileAnimation(tilemapId, x, y)
end

--[[
Applies changes to the tilemap.
@param tilemapId number
]]
Tilemap.applyChanges = function(tilemapId)
	lib.microTilemapApplyChanges(tilemapId)
end

--[[
Frees tilemap.
@param tilemapId number
]]
Tilemap.free = function(tilemapId)
	lib.microTilemapFree(tilemapId)
end

--[[
Frees all tilemaps.
]]
Tilemap.freeAll = function()
	lib.microTilemapFreeAll()
end

--[[
Updates all tilemaps.
@param dt number
]]
Tilemap.update = lib.microTilemapsUpdate

return Tilemap
