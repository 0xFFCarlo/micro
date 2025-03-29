local ffi = require("ffi")
local lib = require("micro/lua/libmicro")

ffi.cdef([[
typedef struct {
  void (*init)();
  void (*update)(float dt);
  void (*free)();
  double time;
} MicroState;

void microStateSet(MicroState state);
void microStateQuit();
void microStateUpdate(float dt);
void microStateFree();
double microStateGetTime();
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
	lib.microStateQuit()
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

--- Returns the current state's time.
--- @return number @The current state time.
function State.getTime()
	return lib.microStateGetTime()
end

return State
