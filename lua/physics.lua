local ffi = require("ffi")
local lib = require("micro/lua/libmicro")

ffi.cdef([[
    // Assumed definition for MicroAABB; adjust if necessary.
    typedef struct {
        float x;
        float y;
        float width;
        float height;
    } MicroAABB;

    typedef struct
    {
      int eid_a, eid_b;
    } MicroCollisionPair;

    typedef void (*MicroWorldCollisionsCb)(MicroCollisionPair *begins,
                                           int begins_count,
                                           MicroCollisionPair *updates,
                                           int updates_count);

    typedef void (*MicroWorldBodiesRemovedCb)(const int *bodyIds, int bodyCount);

    // Physics world functions
    int microPhysicsWorldNew(MicroWorldCollisionsCb collisions_callback,
                             MicroWorldBodiesRemovedCb bodies_removed_callback);
    int microPhysicsWorldUseSpatialHash(int worldId, int cell_size, int cell_count);
    void microPhysicsWorldStep(int worldId, float dt);
    int microPhysicsWorldsCount();
    int microPhysicsWorldGetBodyCount(int worldId);
    int microPhysicsWorldNewCollisionTilemap(int worldId, bool (*is_solid)(const int px, const int py), int tile_size);
    void microPhysicsWorldFree(int worldId);
    void microPhysicsWorldFreeAll();

    // Physics body functions
    int microPhysicsBodyNewCircle(int entityId, int worldId, float cx, float cy,
                                  float radius, float mass, uint8_t isStatic,
                                  uint8_t canRotate, float elasticity, float friction);
    int microPhysicsBodyNewRect(int entityId, int worldId, float cx, float cy,
                                float width, float height, float mass, uint8_t isStatic,
                                uint8_t canRotate, float elasticity, float friction);
    void microPhysicsBodySetFilter(int bodyId, int category, uint32_t mask);
    void microPhysicsBodySetCollisionBeginCallback(int bodyId, void (*callback)(int, int));
    void microPhysicsBodySetCollisionUpdateCallback(int bodyId, void (*callback)(int, int));
    void microPhysicsBodySetCollisionTilemap(int bodyId, int tilemapId);
    void microPhysicsBodyFree(int bodyId);
    void microPhysicsBodySetMass(int bodyId, float mass);
    float microPhysicsBodyGetMass(int bodyId);
    void microPhysicsBodySetPosition(int bodyId, float x, float y);
    void microPhysicsBodySetVelocity(int bodyId, float x, float y);
    void microPhysicsBodySetForce(int bodyId, float x, float y);
    void microPhysicsBodyApplyForce(int bodyId, float x, float y);
    void microPhysicsBodyGetPosition(int bodyId, float *x, float *y);
    void microPhysicsBodyGetVelocity(int bodyId, float *x, float *y);
    void microPhysicsBodyGetForce(int bodyId, float *x, float *y);
    void microPhysicsBodySetRotation(int bodyId, float angle);
    float microPhysicsBodyGetRotation(int bodyId);
    void microPhysicsBodySetSensor(int bodyId, bool is_sensor);
    bool microPhysicsBodyIsSensor(int bodyId);
    bool microPhysicsBodyIsStatic(int bodyId);
    bool microPhysicsBodyIsValid(int bodyId);
    int microPhysicsBodyGetWorldId(int bodyId);
    MicroAABB microPhysicsBodyGetAABB(int bodyId);
]])

---@class PhysicsModule
local Physics = {}

-- WORLD FUNCTIONS

function HandleCollisionsCallback(begins, begins_count, updates, updates_count)
	for i = 0, begins_count - 1 do
		local pair = begins[i]
		if Physics._bodies_collision_begin_cb[pair.eid_a] then
			Physics._bodies_collision_begin_cb[pair.eid_a](pair.eid_a, pair.eid_b)
		end
		if Physics._bodies_collision_begin_cb[pair.eid_b] then
			Physics._bodies_collision_begin_cb[pair.eid_b](pair.eid_b, pair.eid_a)
		end
	end

	for i = 0, updates_count - 1 do
		local pair = updates[i]
		if Physics._bodies_collision_update_cb[pair.eid_a] then
			Physics._bodies_collision_update_cb[pair.eid_a](pair.eid_a, pair.eid_b)
		end
		if Physics._bodies_collision_update_cb[pair.eid_b] then
			Physics._bodies_collision_update_cb[pair.eid_b](pair.eid_b, pair.eid_a)
		end
	end
end

function HandleBodiesRemovedCallback(bodyIds, bodyCount)
	for i = 0, bodyCount - 1 do
		local bodyId = bodyIds[i]
		Physics._bodies_collision_begin_cb[bodyId] = nil
		Physics._bodies_collision_update_cb[bodyId] = nil
	end
end

Physics._HandleCollisionsCallback = ffi.cast("MicroWorldCollisionsCb", HandleCollisionsCallback)
Physics._HandleBodiesRemovedCallback = ffi.cast("MicroWorldBodiesRemovedCb", HandleBodiesRemovedCallback)
Physics._bodies_collision_begin_cb = {}
Physics._bodies_collision_update_cb = {}

--- Creates a new physics world.
--- @return number worldId
function Physics.worldNew()
	return lib.microPhysicsWorldNew(Physics._HandleCollisionsCallback, Physics._HandleBodiesRemovedCallback)
end

--- Creates a new physics world with a spatial hash.
--- @param worldId number
--- @param cell_size number
--- @param cell_count number
--- @return number result
function Physics.worldUseSpatialHash(worldId, cell_size, cell_count)
	return lib.microPhysicsWorldUseSpatialHash(worldId, cell_size, cell_count)
end

--- Steps the physics simulation for the specified world.
--- @param worldId number
--- @param dt number Delta time.
function Physics.worldStep(worldId, dt)
	lib.microPhysicsWorldStep(worldId, dt)
end

--- Returns the number of physics worlds.
--- @return number count
function Physics.worldsCount()
	return lib.microPhysicsWorldsCount()
end

--- Returns the number of bodies in a physics world.
--- @param worldId number
--- @return number count
function Physics.worldGetBodyCount(worldId)
	return lib.microPhysicsWorldGetBodyCount(worldId)
end

--- Creates a new collision tilemap in a physics world.
--- @param worldId number
--- @param is_solid function Callback function: function(px: number, py: number): boolean.
--- @param tile_size number
--- @return number tilemapId
function Physics.worldNewCollisionTilemap(worldId, is_solid, tile_size)
	local c_is_solid = ffi.cast("bool (*)(const int, const int)", is_solid)
	return lib.microPhysicsWorldNewCollisionTilemap(worldId, c_is_solid, tile_size)
end

--- Frees a physics world.
--- @param worldId number
function Physics.worldFree(worldId)
	lib.microPhysicsWorldFree(worldId)
end

--- Frees all physics worlds.
function Physics.worldFreeAll()
	lib.microPhysicsWorldFreeAll()
end

-- BODY FUNCTIONS

--- Creates a new circular physics body.
--- @param entityId number
--- @param worldId number
--- @param cx number Center x coordinate.
--- @param cy number Center y coordinate.
--- @param radius number
--- @param mass number
--- @param isStatic number (0 or 1)
--- @param canRotate number (0 or 1)
--- @param elasticity number
--- @param friction number
--- @return number bodyId
function Physics.bodyNewCircle(entityId, worldId, cx, cy, radius, mass, isStatic, canRotate, elasticity, friction)
	return lib.microPhysicsBodyNewCircle(
		entityId,
		worldId,
		cx,
		cy,
		radius,
		mass,
		isStatic,
		canRotate,
		elasticity,
		friction
	)
end

--- Creates a new rectangular physics body.
--- @param entityId number
--- @param worldId number
--- @param cx number Center x coordinate.
--- @param cy number Center y coordinate.
--- @param width number
--- @param height number
--- @param mass number
--- @param isStatic number (0 or 1)
--- @param canRotate number (0 or 1)
--- @param elasticity number
--- @param friction number
--- @return number bodyId
function Physics.bodyNewRect(entityId, worldId, cx, cy, width, height, mass, isStatic, canRotate, elasticity, friction)
	return lib.microPhysicsBodyNewRect(
		entityId,
		worldId,
		cx,
		cy,
		width,
		height,
		mass,
		isStatic,
		canRotate,
		elasticity,
		friction
	)
end

--- Sets the collision filter for a physics body.
--- @param bodyId number
--- @param category number
--- @param mask number
function Physics.bodySetFilter(bodyId, category, mask)
	lib.microPhysicsBodySetFilter(bodyId, category, mask)
end

--- Sets the collision begin callback for a physics body.
--- @param bodyId number
--- @param callback function Callback function: function(body1: number, body2: number):void
function Physics.bodySetCollisionBeginCallback(entityId, collisionBeginCb)
	Physics._bodies_collision_begin_cb[entityId] = collisionBeginCb
end

--- Sets the collision update callback for a physics body.
--- @param bodyId number
--- @param callback function Callback function: function(body1: number, body2: number):void
function Physics.bodySetCollisionUpdateCallback(entityId, collisionUpdateCb)
	Physics._bodies_collision_update_cb[entityId] = collisionUpdateCb
end

--- Sets the collision tilemap for a physics body.
--- @param bodyId number
--- @param tilemapId number
function Physics.bodySetCollisionTilemap(bodyId, tilemapId)
	lib.microPhysicsBodySetCollisionTilemap(bodyId, tilemapId)
end

--- Frees a physics body.
--- @param bodyId number
function Physics.bodyFree(bodyId)
	lib.microPhysicsBodyFree(bodyId)
end

--- Sets the mass of a physics body.
--- @param bodyId number
--- @param mass number
function Physics.bodySetMass(bodyId, mass)
	lib.microPhysicsBodySetMass(bodyId, mass)
end

--- Returns the mass of a physics body.
--- @param bodyId number
--- @return number mass
function Physics.bodyGetMass(bodyId)
	return lib.microPhysicsBodyGetMass(bodyId)
end

--- Sets the position of a physics body.
--- @param bodyId number
--- @param x number
--- @param y number
function Physics.bodySetPosition(bodyId, x, y)
	lib.microPhysicsBodySetPosition(bodyId, x, y)
end

--- Sets the velocity of a physics body.
--- @param bodyId number
--- @param x number
--- @param y number
function Physics.bodySetVelocity(bodyId, x, y)
	lib.microPhysicsBodySetVelocity(bodyId, x, y)
end

--- Sets the force of a physics body.
--- @param bodyId number
--- @param x number
--- @param y number
function Physics.bodySetForce(bodyId, x, y)
	lib.microPhysicsBodySetForce(bodyId, x, y)
end

--- Applies a force to a physics body.
--- @param bodyId number
--- @param x number
--- @param y number
function Physics.bodyApplyForce(bodyId, x, y)
	lib.microPhysicsBodyApplyForce(bodyId, x, y)
end

--- Retrieves the position of a physics body.
--- @param bodyId number
--- @return number x, number y
function Physics.bodyGetPosition(bodyId)
	local x = ffi.new("float[1]")
	local y = ffi.new("float[1]")
	lib.microPhysicsBodyGetPosition(bodyId, x, y)
	return x[0], y[0]
end

--- Retrieves the velocity of a physics body.
--- @param bodyId number
--- @return number x, number y
function Physics.bodyGetVelocity(bodyId)
	local x = ffi.new("float[1]")
	local y = ffi.new("float[1]")
	lib.microPhysicsBodyGetVelocity(bodyId, x, y)
	return x[0], y[0]
end

--- Retrieves the force of a physics body.
--- @param bodyId number
--- @return number x, number y
function Physics.bodyGetForce(bodyId)
	local x = ffi.new("float[1]")
	local y = ffi.new("float[1]")
	lib.microPhysicsBodyGetForce(bodyId, x, y)
	return x[0], y[0]
end

--- Sets the rotation of a physics body.
--- @param bodyId number
--- @param angle number
function Physics.bodySetRotation(bodyId, angle)
	lib.microPhysicsBodySetRotation(bodyId, angle)
end

--- Retrieves the rotation of a physics body.
--- @param bodyId number
--- @return number angle
function Physics.bodyGetRotation(bodyId)
	return lib.microPhysicsBodyGetRotation(bodyId)
end

--- Sets whether a physics body is a sensor.
--- @param bodyId number
--- @param is_sensor boolean
function Physics.bodySetSensor(bodyId, is_sensor)
	lib.microPhysicsBodySetSensor(bodyId, is_sensor)
end

--- Checks if a physics body is a sensor.
--- @param bodyId number
--- @return boolean
function Physics.bodyIsSensor(bodyId)
	return lib.microPhysicsBodyIsSensor(bodyId)
end

--- Checks if a physics body is static.
--- @param bodyId number
--- @return boolean
function Physics.bodyIsStatic(bodyId)
	return lib.microPhysicsBodyIsStatic(bodyId)
end

--- Checks if a physics body is valid.
--- @param bodyId number
--- @return boolean
function Physics.bodyIsValid(bodyId)
	return lib.microPhysicsBodyIsValid(bodyId)
end

--- Retrieves the world id associated with a physics body.
--- @param bodyId number
--- @return number worldId
function Physics.bodyGetWorldId(bodyId)
	return lib.microPhysicsBodyGetWorldId(bodyId)
end

--- Retrieves the axis-aligned bounding box (AABB) of a physics body.
--- @param bodyId number
--- @return table A table with keys: x, y, width, height.
function Physics.bodyGetAABB(bodyId)
	local aabb = lib.microPhysicsBodyGetAABB(bodyId)
	return { x = aabb.x, y = aabb.y, width = aabb.width, height = aabb.height }
end

return Physics
