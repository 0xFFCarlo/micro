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

    // Get number of deleted entities in the last frame.
    int microECSGetDeletedEntitiesCount();

    // Return the entities ids that were deleted in the last frame.
    void microECSGetDeletedEntities(int *entities, int *size);
]])

---@class ECSModule
local ECS = {}

ECS.entitiesData = {}
ECS.updateComponents = {}

--- Creates a new entity.
--- @param data any|nil Table with the entity data.
--- @param freeCb fun(entityId: number)|nil Callback to free the entity data.
--- @return number Entity id.
function ECS.newEntity(data, freeCb)
	local id = lib.microECSEntityNew(nil, nil)
	ECS.entitiesData[id + 1] = { data = data, free = freeCb }
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
--- @return any Pointer to the entity data.
function ECS.getData(entityId)
	if ECS.entitiesData[entityId + 1] == nil then
		return nil
	end
	return ECS.entitiesData[entityId + 1].data
end

--- Sets the private data pointer for an entity.
--- @param entityId number The entity id.
--- @param data any Pointer to the new data.
function ECS.setData(entityId, data)
	ECS.entitiesData[entityId + 1].data = data
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

function ECS.getDeletedEntitiesCount()
	return lib.microECSGetDeletedEntitiesCount()
end

function ECS.getDeletedEntities()
	local entities = ffi.new("int[?]", ECS.getDeletedEntitiesCount())
	local size = ffi.new("int[1]")
	lib.microECSGetDeletedEntities(entities, size)
	return entities, size[0]
end

function ECS.handleCleanupEntities()
	local entities, size = ECS.getDeletedEntities()
	for i = 0, size - 1 do
		local entityId = entities[i]
		if ECS.entitiesData[entityId + 1] ~= nil then
			local freeCb = ECS.entitiesData[entityId + 1].free
			if freeCb ~= nil then
				freeCb(entityId)
			end
			ECS.entitiesData[entityId + 1] = nil
		end
		ECS.updateComponents[entityId + 1] = nil
	end
end

return ECS
