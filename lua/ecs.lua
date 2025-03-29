local ffi = require("ffi")
local lib = require("micro/lua/libmicro")

ffi.cdef([[
    // Create new entity and return its id.
    int microECSEntityNew(void *data, void (*free)(int));

    // Request to remove entity from the world.
    void microECSEntityRemove(int entityId);

    // Check if entity is alive.
    int microECSEntityIsAlive(int entityId);

    // Get entity private data pointer.
    void *microECSEntityGetData(int entityId);

    // Set entity private data pointer.
    void microECSEntitySetData(int entityId, void *data);

    // Set entity free data function.
    void microECSEntitySetFreeData(int entityId, void (*free)(int));

    // Add component to entity.
    void microECSEntityAddComponent(int entityId, const int componentTypeId, void *data);

    // Remove component from entity.
    void microECSEntityRemoveComponent(int entityId, const int componentTypeId);

    // Get component owned by entity.
    void *microECSEntityGetComponent(int entityId, const int componentTypeId);

    // Check if entity has component.
    int microECSEntityHasComponent(int entityId, const int componentTypeId);

    // Get number of entities in the world.
    int microECSEntitiesCount();

    // Free all entities.
    void microECSEntityFreeAll();

    // Allocate memory for components.
    void microECSAllocateComponents();
]])

---@class ECSModule
local ECS = {}

local tableRegistry = {}
local nextKey = 1LL -- Use a 64-bit integer for the key

--- Registers a Lua table and returns a unique 64-bit key.
---@param tbl table
---@return number key A unique 64-bit key.
local function registerTable(tbl)
  local key = nextKey
  nextKey = nextKey + 1
  tableRegistry[key] = tbl
  return key
end

--- Unregisters a table using its key.
---@param key number
local function unregisterTable(key)
  if key == nil then
    return
  end
  tableRegistry[key] = nil
end

--- Frees the entity data.
---@param entityId number
local function entityFree(entityId)
  local dataKey = ECS.getData(entityId)
  unregisterTable(dataKey)
end

--- Creates a new entity.
--- @param data any Table with the entity data.
--- @return number Entity id.
function ECS.newEntity(data)
  -- Cast the Lua free callback to a C function pointer.
  local free_cb = ffi.cast("void (*)(int)", entityFree)
  local dataKey
  if data == nil then
    dataKey = nil
  else
    dataKey = registerTable(data)
  end
  local id = lib.microECSEntityNew(dataKey, free_cb)
  return id
end

--- Removes an entity from the world.
--- @param entityId number The id of the entity to remove.
function ECS.removeEntity(entityId)
  lib.microECSEntityRemove(entityId)
end

--- Checks if an entity is alive.
--- @param entityId number The entity id.
--- @return boolean True if the entity is alive, false otherwise.
function ECS.isAlive(entityId)
  return lib.microECSEntityIsAlive(entityId) ~= 0
end

--- Gets the private data pointer for an entity.
--- @param entityId number The entity id.
--- @return number to the entity data.
function ECS.getData(entityId)
  return lib.microECSEntityGetData(entityId)
end

--- Sets the private data pointer for an entity.
--- @param entityId number The entity id.
--- @param data any Pointer to the new data.
function ECS.setData(entityId, data)
  lib.microECSEntitySetData(entityId, data)
end

--- Sets the free data callback for an entity.
--- @param entityId number The entity id.
--- @param free fun(entityId: number) Callback to free the entity data.
function ECS.setFreeData(entityId, free)
  local free_cb = ffi.cast("void (*)(int)", free)
  lib.microECSEntitySetFreeData(entityId, free_cb)
  ECS._freeCallbacks = ECS._freeCallbacks or {}
  ECS._freeCallbacks[entityId] = free_cb
end

--- Adds a component to an entity.
--- @param entityId number The entity id.
--- @param componentTypeId number The component type identifier.
--- @param data any Pointer to the component data.
function ECS.addComponent(entityId, componentTypeId, data)
  lib.microECSEntityAddComponent(entityId, componentTypeId, data)
end

--- Removes a component from an entity.
--- @param entityId number The entity id.
--- @param componentTypeId number The component type identifier.
function ECS.removeComponent(entityId, componentTypeId)
  lib.microECSEntityRemoveComponent(entityId, componentTypeId)
end

--- Gets a component from an entity.
--- @param entityId number The entity id.
--- @param componentTypeId number The component type identifier.
--- @return userdata Pointer to the component data.
function ECS.getComponent(entityId, componentTypeId)
  return lib.microECSEntityGetComponent(entityId, componentTypeId)
end

--- Checks if an entity has a specific component.
--- @param entityId number The entity id.
--- @param componentTypeId number The component type identifier.
--- @return boolean True if the entity has the component, false otherwise.
function ECS.hasComponent(entityId, componentTypeId)
  return lib.microECSEntityHasComponent(entityId, componentTypeId) ~= 0
end

--- Returns the total number of entities in the world.
--- @return number Entity count.
function ECS.getEntitiesCount()
  return lib.microECSEntitiesCount()
end

--- Frees all entities.
function ECS.freeAll()
  lib.microECSEntityFreeAll()
end

--- Allocates memory for components.
function ECS.allocateComponents()
  lib.microECSAllocateComponents()
end

return ECS
