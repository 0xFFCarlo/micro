local ffi = require("ffi")
local lib = require("micro/lua/libmicro")

ffi.cdef([[
typedef struct {
  void (*init)();
  void (*update)(float dt);
  void (*free)();
  double time;
} MicroState;

int microInit(MicroState bootState);
int microUpdate(int max_fps);
int microQuit();
]])

local Micro = {}

local _bootState = nil

--- Initializes the system with the given boot state.
--- @param stateInit fun()
--- @param stateUpdate fun(dt: number)
--- @param stateFree fun()
function Micro.init(stateInit, stateUpdate, stateFree)
	_bootState = ffi.new("MicroState")
	_bootState.init = stateInit
	_bootState.update = stateUpdate
	_bootState.free = stateFree
	_bootState.time = 0.0
	local rc = lib.microInit(_bootState)
	if rc ~= 0 then
		error("Failed to initialize Micro")
		os.exit(1)
	end
end

--- Updates the system with the specified maximum FPS.
--- @param max_fps number The maximum frames per second.
--- @return number The result of the update function.
function Micro.update(max_fps)
	return lib.microUpdate(max_fps)
end

--- Frees the resources used by the system.
--- @return number 0 on success, non-zero on error.
function Micro.quit()
	return lib.microQuit()
end

return Micro
