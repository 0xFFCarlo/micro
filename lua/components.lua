---@class CListener
---@field max_distance number

---@class CSoundSource
---@field source_id number

local ffi = require("ffi")
local lib = require("micro/lua/libmicro")
local ECS = require("micro/lua/ecs")

ffi.cdef([[
  typedef struct
  {
    float max_distance;
  } CListener;

  void CmpAddListener(int entity_id, float max_distance);
  CListener *CmpGetListener(int entity_id);

  typedef struct
  {
    int source_id;
  } CSoundSource;

  void CmpAddSoundSource(int entity_id, const char *mainChannelTag, float gain);
  CSoundSource *CmpGetSoundSource(int entity_id);

  // Sprite
  typedef struct
  {
    uint16_t textureId;
    int16_t tx, ty, tw, th;
  } CSprite;
  void CmpAddSprite(int entity_id, int textureId, int tx, int ty, int tw,
                    int th);
  CSprite *CmpGetSprite(int entity_id);

  // Sprite buffer (Mesh)
  typedef struct {
    uint32_t VAOId;
  } CMesh;
  void CmpAddMesh(int entity_id, int shaderId, int textureId, int vertexCount,
                  int instanceCount, const void *attributes, int attributesCount);
  CMesh *CmpGetMesh(int entity_id);

  // Text
  typedef struct {
    uint8_t fontId;
    float scale;
    float lineSpacing;
    uint32_t alignment;
    uint32_t maxLineWidth;
    char *text;
  } CText;
  void CmpAddText(int entity_id, uint8_t fontId, float scale, float lineSpacing,
                  uint32_t alignment, uint32_t maxLineWidth, char *text);
  CText *CmpGetText(int entity_id);

  // Color
  typedef struct {
    unsigned char r, g, b, a;
  } CColor;
  void CmpAddColor(int entity_id, unsigned char r, unsigned char g,
                   unsigned char b, unsigned char a);
  CColor *CmpGetColor(int entity_id);

  // Layer (Drawable)
  typedef struct {
    uint8_t layerId;
    bool visible;
  } CDrawable;
  void CmpAddDrawable(int entity_id, uint8_t layerId, bool visible);
  CDrawable *CmpGetDrawable(int entity_id);

  void CmpAddHud(int entity_id);

  // Animation
  typedef struct {
    int animationId;
    uint16_t frameId;
    float duration;
    bool flipX;
    bool flipY;
    bool reverse;
    float animationTime;
  } CAnimation;
  void CmpAddAnimation(int entity_id, int animationId, float duration, bool flipX,
                       bool flipY, bool reverse);
  CAnimation *CmpGetAnimation(int entity_id);

  // ShadedCanvas
  typedef struct {
    int width, height;
    int shaderId;
    int canvasId;
    unsigned char needsUpdate;
  } CShadedCanvas;
  void CmpAddShaderCanvas(int entity_id, int width, int height, int shaderId,
                          int canvasId);
  CShadedCanvas *CmpGetShadedCanvas(int entity_id);

  // LockOnView
  typedef struct {
    bool followRotation;
    bool hasBoundaries;
    float minX, minY, maxX, maxY;
  } CLockOnView;
  void CmpAddLockOnView(int entity_id, bool followRotation, bool hasBoundaries,
                        float minX, float minY, float maxX, float maxY);
  CLockOnView *CmpGetLockOnView(int entity_id);

  // ParticleEmitter
  typedef struct {
    uint16_t emitterId;
    uint16_t offsetX, offsetY;
  } CParticleEmitter;
  void CmpAddParticleEmitter(int entity_id, uint16_t emitterId, uint16_t offsetX, uint16_t offsetY);
  CParticleEmitter *CmpGetParticleEmitter(int entity_id);

  // Light source
  typedef struct {
    int lightId;
  } CLightSource;
  void CmpAddLightSource(int entity_id, float intensity, float radius);
  CLightSource *CmpGetLightSource(int entity_id);

  typedef struct
  {
    double x, y;
  } CPosition;

  void CmpAddPosition(int entity_id, double x, double y);
  CPosition *CmpGetPosition(int entity_id);

  // Parent
  typedef struct
  {
    int parent_eid;
  } CParent;

  void CmpAddParent(int entity_id, int parent_eid);
  CParent *CmpGetParent(int entity_id);

  // Transform
  typedef struct
  {
    int width, height;
    float originX, originY;
    float rotation;
  } CTransform;

  void CmpAddTransform(int entity_id, int width, int height, float originX,
                       float originY, float rotation);
  CTransform *CmpGetTransform(int entity_id);

  // Body
  typedef struct
  {
    int body_id;
  } CBody;

  void CmpAddBodyCircle(int entity_id, float cx, float cy, float radius,
                        float mass, int isStatic, uint8_t canRotate,
                        float elasticity, float friction);
  void CmpAddBodyRect(int entity_id, float cx, float cy, float width,
                      float height, float mass, int isStatic, uint8_t canRotate,
                      float elasticity, float friction);
  CBody *CmpGetBody(int entity_id);

  // Follow an entity
  typedef struct
  {
    uint32_t target_entity_id;
    uint8_t lock_rot;
    int32_t offset_x;
    int32_t offset_y;
  } CFollow;

  void CmpAddFollow(int entity_id, uint32_t target_entity_id, uint8_t lock_rot,
                    int32_t offset_x, int32_t offset_y);
  CFollow *CmpGetFollow(int entity_id);

  typedef struct
  {
    void (*update)(int, float);
  } CUpdate;

  void CmpAddUpdate(int entity_id, void (*update)(int, float));
  CUpdate *CmpGetUpdate(int entity_id);

  typedef struct
  {
  } CScriptedUpdate;

  void CmpAddScriptedUpdate(int entity_id);
  CScriptedUpdate *CmpGetScriptedUpdate(int entity_id);
  typedef void (*UpdateHandlerType)(int* eids, int count, float dt);
  void CmpSetScriptedUpdateCb(UpdateHandlerType cb);

  typedef struct
  {
    float lifetime;
    float max_lifetime;
  } CLifetime;

  void CmpAddLifetime(int entity_id, float lifetime);
  CLifetime *CmpGetLifetime(int entity_id);

  typedef struct
  {
    int category;
  } CEntityCategory;

  void CmpAddEntityCategory(int entity_id, int category);
  CEntityCategory *CmpGetEntityCategory(int entity_id);

  typedef struct
  {
    const char *name;
  } CName;

  void CmpAddName(int entity_id, const char *name);
  CName *CmpGetName(int entity_id);

  typedef struct CHealth {
      unsigned int max;
      unsigned int current;
  } CHealth;
  void CmpAddHealth(int eid, unsigned int max, unsigned int current);
  CHealth* CmpGetHealth(int eid);
]])

-- Components functions
local Cmp = {}

--- Add a listener component to an entity.
--- @param entity_id number
--- @param max_distance number
function Cmp.addListener(entity_id, max_distance)
	lib.CmpAddListener(entity_id, max_distance)
end

--- Get the listener component for an entity.
--- @param entity_id number
--- @return CListener
function Cmp.getListener(entity_id)
	return lib.CmpGetListener(entity_id)
end

--- Add a sound source component to an entity.
--- @param entity_id number
--- @param mainChannelTag string
--- @param gain number
function Cmp.addSoundSource(entity_id, mainChannelTag, gain)
	lib.CmpAddSoundSource(entity_id, mainChannelTag, gain)
end

--- Get the sound source component for an entity.
--- @param entity_id number
--- @return CSoundSource
function Cmp.getSoundSource(entity_id)
	return lib.CmpGetSoundSource(entity_id)
end

function Cmp.addSprite(entity_id, textureId, tx, ty, tw, th)
	lib.CmpAddSprite(entity_id, textureId, tx, ty, tw, th)
end

function Cmp.getSprite(entity_id)
	local cmp = lib.CmpGetSprite(entity_id)
	assert(cmp ~= nil, "Sprite component not found")
	return cmp
end

function Cmp.addMesh(entity_id, shaderId, textureId, vertexCount, instanceCount, attributes, attributesCount)
	lib.CmpAddMesh(entity_id, shaderId, textureId, vertexCount, instanceCount, attributes, attributesCount)
end

function Cmp.getMesh(entity_id)
	return lib.CmpGetMesh(entity_id)
end

function Cmp.addText(entity_id, fontId, scale, lineSpacing, alignment, maxLineWidth, text)
	lib.CmpAddText(entity_id, fontId, scale, lineSpacing, alignment, maxLineWidth, text)
end

function Cmp.getText(entity_id)
	return lib.CmpGetText(entity_id)
end

function Cmp.addColor(entity_id, r, g, b, a)
	r = r or 255
	g = g or 255
	b = b or 255
	a = a or 255
	lib.CmpAddColor(entity_id, r, g, b, a)
end

function Cmp.getColor(entity_id)
	return lib.CmpGetColor(entity_id)
end

--- Add a drawable component to an entity.
--- @param entity_id number
--- @param layerId number
--- @param visible boolean|nil
function Cmp.addDrawable(entity_id, layerId, visible)
	visible = visible or true
	lib.CmpAddDrawable(entity_id, layerId, visible)
end

function Cmp.getDrawable(entity_id)
	return lib.CmpGetDrawable(entity_id)
end

function Cmp.addHud(entity_id)
	lib.CmpAddHud(entity_id)
end

function Cmp.getHud(entity_id)
	return lib.CmpGetHud(entity_id)
end

--- Add an animation component to an entity.
--- @param entity_id number
--- @param animationId number
--- @param duration number
--- @param flipX boolean|nil
--- @param flipY boolean|nil
--- @param reverse boolean|nil
function Cmp.addAnimation(entity_id, animationId, duration, flipX, flipY, reverse)
	flipX = flipX or false
	flipY = flipY or false
	reverse = reverse or false
	lib.CmpAddAnimation(entity_id, animationId, duration, flipX, flipY, reverse)
end

function Cmp.getAnimation(entity_id)
	return lib.CmpGetAnimation(entity_id)
end

function Cmp.addShadedCanvas(entity_id, width, height, shaderId, canvasId)
	lib.CmpAddShaderCanvas(entity_id, width, height, shaderId, canvasId)
end

function Cmp.getShadedCanvas(entity_id)
	return lib.CmpGetShadedCanvas(entity_id)
end

--- Add a lock on view component to an entity.
--- @param entity_id number
--- @param followRotation boolean|nil
--- @param hasBoundaries boolean|nil
--- @param minX number|nil
--- @param minY number|nil
--- @param maxX number|nil
--- @param maxY number|nil
function Cmp.addLockOnView(entity_id, followRotation, hasBoundaries, minX, minY, maxX, maxY)
	followRotation = followRotation or false
	hasBoundaries = hasBoundaries or false
	minX = minX or 0
	minY = minY or 0
	maxX = maxX or 1000
	maxY = maxY or 1000
	lib.CmpAddLockOnView(entity_id, followRotation, hasBoundaries, minX, minY, maxX, maxY)
end

function Cmp.getLockOnView(entity_id)
	return lib.CmpGetLockOnView(entity_id)
end

function Cmp.addParticleEmitter(entity_id, emitterId, offsetX, offsetY)
	lib.CmpAddParticleEmitter(entity_id, emitterId, offsetX, offsetY)
end

function Cmp.getParticleEmitter(entity_id)
	return lib.CmpGetParticleEmitter(entity_id)
end

function Cmp.addLightSource(entity_id, intensity, radius)
	lib.CmpAddLightSource(entity_id, intensity, radius)
end

function Cmp.getLightSource(entity_id)
	return lib.CmpGetLightSource(entity_id)
end

function Cmp.addPosition(entity_id, x, y)
	lib.CmpAddPosition(entity_id, x, y)
end

function Cmp.getPosition(entity_id)
	local cmp = lib.CmpGetPosition(entity_id)
	assert(cmp ~= nil, "Position component not found")
	return cmp
end

function Cmp.addParent(entity_id, parent_eid)
	lib.CmpAddParent(entity_id, parent_eid)
end

function Cmp.getParent(entity_id)
	local cmp = lib.CmpGetParent(entity_id)
	assert(cmp ~= nil, "Parent component not found")
	return cmp
end

--- Add a transform component to an entity.
--- @param entity_id number
--- @param width number
--- @param height number
--- @param originX number|nil
--- @param originY number|nil
--- @param rotation number|nil
function Cmp.addTransform(entity_id, width, height, originX, originY, rotation)
	originX = originX or 0.5
	originY = originY or 0.5
	rotation = rotation or 0.0
	lib.CmpAddTransform(entity_id, width, height, originX, originY, rotation)
end

function Cmp.getTransform(entity_id)
	return lib.CmpGetTransform(entity_id)
end

function Cmp.addBodyCircle(entity_id, cx, cy, radius, mass, isStatic, canRotate, elasticity, friction)
	lib.CmpAddBodyCircle(entity_id, cx, cy, radius, mass, isStatic, canRotate, elasticity, friction)
end

function Cmp.addBodyRect(entity_id, cx, cy, width, height, mass, isStatic, canRotate, elasticity, friction)
	lib.CmpAddBodyRect(entity_id, cx, cy, width, height, mass, isStatic, canRotate, elasticity, friction)
end

function Cmp.getBody(entity_id)
	return lib.CmpGetBody(entity_id)
end

function Cmp.addFollow(entity_id, target_entity_id, lock_rot, offset_x, offset_y)
	lib.CmpAddFollow(entity_id, target_entity_id, lock_rot, offset_x, offset_y)
end

function Cmp.getFollow(entity_id)
	return lib.CmpGetFollow(entity_id)
end

--- Adds an update component to an entity.
--- @param entity_id number The entity identifier.
--- @param update fun(entity_id:number, delta: number) The update callback.
Cmp.updateComponents = {}
function Cmp.addUpdate(entity_id, update)
	assert(type(update) == "function", "update must be a function")
	assert(ECS.isAlive(entity_id), "Entity is not alive")
	Cmp.updateComponents[entity_id] = {}
	Cmp.updateComponents[entity_id].update = update
	lib.CmpAddScriptedUpdate(entity_id)
end

-- Wrap the Lua function in ffi.cast
local function ScriptedUpdateHandler(eids, count, dt)
	local eid_array = ffi.cast("int*", eids)

	for _, updateComponent in pairs(Cmp.updateComponents) do
		updateComponent.is_in_use = false
	end

	for i = 0, count - 1 do
		local eid = eid_array[i]
		local updateComponent = Cmp.updateComponents[eid]
		updateComponent.is_in_use = true
		if updateComponent then
			if ECS.isAlive(eid) and updateComponent.update ~= nil then
				updateComponent.update(eid, dt)
				-- print("Updated entity: " .. eid .. " with dt: " .. dt)
			end
		end
	end

	for eid, updateComponent in pairs(Cmp.updateComponents) do
		if Cmp.updateComponents[eid].is_in_use == false then
			Cmp.updateComponents[eid] = nil
		end
	end
end

-- Cast Lua function to C function pointer
Cmp._update_cb = ffi.cast("UpdateHandlerType", ScriptedUpdateHandler)

-- Pass it to C
lib.CmpSetScriptedUpdateCb(Cmp._update_cb)

--- @class CUpdate
--- @field update fun()

--- @class CLifetime
--- @field lifetime number
--- @field max_lifetime number

--- Retrieves the update component for an entity.
--- @param entity_id number The entity identifier.
--- @return CUpdate A pointer to the update component.
function Cmp.getUpdate(entity_id)
	return Cmp.updateComponents[entity_id]
end

--- Adds a lifetime component to an entity.
--- @param entity_id number The entity identifier.
--- @param lifetime number The lifetime value.
function Cmp.addLifetime(entity_id, lifetime)
	lib.CmpAddLifetime(entity_id, lifetime)
end

--- Retrieves the lifetime component for an entity.
--- @param entity_id number The entity identifier.
--- @return CLifetime A pointer to the lifetime component.
function Cmp.getLifetime(entity_id)
	local cmp = lib.CmpGetLifetime(entity_id)
	assert(cmp ~= nil, "Lifetime component not found")
	return cmp
end

--- Adds an entity category component.
--- @param entity_id number The entity identifier.
--- @param category number The category value.
function Cmp.addEntityCategory(entity_id, category)
	lib.CmpAddEntityCategory(entity_id, category)
end

--- Retrieves the entity category component.
--- @param entity_id number The entity identifier.
--- @return CEntityCategory A pointer to the entity category component.
function Cmp.getEntityCategory(entity_id)
	return lib.CmpGetEntityCategory(entity_id)
end

--- Adds a name component to an entity.
--- @param entity_id number The entity identifier.
--- @param name string The name string.
function Cmp.addName(entity_id, name)
	lib.CmpAddName(entity_id, name)
end

--- Retrieves the name component for an entity.
--- @param entity_id number The entity identifier.
--- @return CName A pointer to the name component.
function Cmp.getName(entity_id)
	return lib.CmpGetName(entity_id)
end

--- Adds a health component to an entity.
--- @param eid number The entity identifier.
--- @param max number The maximum health.
--- @param current number The current health.
function Cmp.addHealth(eid, max, current)
	lib.CmpAddHealth(eid, max, current)
end

--- Retrieves the health component for an entity.
--- @param eid number The entity identifier.
--- @return CHealth A pointer to the health component.
function Cmp.getHealth(eid)
	return lib.CmpGetHealth(eid)
end

return Cmp
