local ffi = require("ffi")
local lib = require("micro/lua/libmicro")

ffi.cdef([[
typedef struct {
  void (*init)();
  void (*update)(float dt);
  void (*free)();
  double time;
  float dt;
} MicroState;

void microStateSet(MicroState state);
void microStateUpdate(float dt);
void microStateFree();
double microStateGetLastBusyTime();
double microStateGetTime();
float microStateGetDeltaTime();
]])

local State = {}
local _currentState = nil

--- Sets the current state with the provided MicroState.
--- @param stateInit fun()
--- @param stateUpdate fun(dt: number)
--- @param stateFree fun()
function State.set(stateInit, stateUpdate, stateFree)
	_currentState = ffi.new("MicroState")
	_currentState.init = stateInit
	_currentState.update = stateUpdate
	_currentState.free = stateFree
	_currentState.time = 0.0
	lib.microStateSet(_currentState)
end

--- Signals the state to quit.
function State.quit()
	lib.microStateFree()
end

--- Updates the current state with the given delta time.
--- @param dt number @Delta time in seconds.
function State.update(dt)
	lib.microStateUpdate(dt)
end

--- Frees the resources associated with the current state.
function State.free()
	lib.microStateFree()
end

--- Returns the last busy time of the current state.
--- @return number @The last busy time in seconds.
function State.getLastBusyTime()
  return lib.microStateGetLastBusyTime()
end

--- Returns the current state's time.
--- @return number @The current state time.
function State.getTime()
	return lib.microStateGetTime()
end

--- Returns the current state's delta time.
--- @return number @The current state delta time in seconds.
function State.getDeltaTime()
	return lib.microStateGetDeltaTime()
end

return State
