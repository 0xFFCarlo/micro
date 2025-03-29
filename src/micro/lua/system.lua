local ffi = require("ffi")
local lib = require("src/lua/libmicro")

-- Declare the functions we need from the shared library.
ffi.cdef([[
int microSystemGetKey(int scancode);
void microSystemGetMousePos(int *x, int *y);
void microSystemSetMousePos(int x, int y);
void microSystemGetWindowSize(int *width, int *height);
void microSystemShowCursor(bool show);
void microSystemFocusWindow();
bool microSystemIsGamepadConnected();
]])

local System = {}

-- Key codes.
-- @class
-- @name System.Key
System.Key = {
	UNKNOWN = 0,
	A = 4,
	B = 5,
	C = 6,
	D = 7,
	E = 8,
	F = 9,
	G = 10,
	H = 11,
	I = 12,
	J = 13,
	K = 14,
	L = 15,
	M = 16,
	N = 17,
	O = 18,
	P = 19,
	Q = 20,
	R = 21,
	S = 22,
	T = 23,
	U = 24,
	V = 25,
	W = 26,
	X = 27,
	Y = 28,
	Z = 29,
	NUM_1 = 30,
	NUM_2 = 31,
	NUM_3 = 32,
	NUM_4 = 33,
	NUM_5 = 34,
	NUM_6 = 35,
	NUM_7 = 36,
	NUM_8 = 37,
	NUM_9 = 38,
	NUM_0 = 39,
	RETURN = 40,
	ESCAPE = 41,
	BACKSPACE = 42,
	TAB = 43,
	SPACE = 44,
	MINUS = 45,
	EQUALS = 46,
	LEFTBRACKET = 47,
	RIGHTBRACKET = 48,
	BACKSLASH = 49,
	NONUSHASH = 50,
	SEMICOLON = 51,
	APOSTROPHE = 52,
	GRAVE = 53,
	COMMA = 54,
	PERIOD = 55,
	SLASH = 56,
	RIGHT = 79,
	LEFT = 80,
	DOWN = 81,
	UP = 82,
}

-- Get the state of a key.
-- @param key number The key code.
-- @return boolean True if the key is pressed, false otherwise.
function System.getKey(key)
	return (lib.microSystemGetKey(key) == 1)
end

-- Get the mouse position.
-- @return number, number The x and y coordinates.
function System.getMousePos()
	local x = ffi.new("int[1]")
	local y = ffi.new("int[1]")
	lib.microSystemGetMousePos(x, y)
	return x[0], y[0]
end

-- Set the mouse position.
-- @param x number The x coordinate.
-- @param y number The y coordinate.
function System.setMousePos(x, y)
	lib.microSystemSetMousePos(x, y)
end

-- Get the window size.
-- @return number, number The window width and height.
function System.getWindowSize()
	local width = ffi.new("int[1]")
	local height = ffi.new("int[1]")
	lib.microSystemGetWindowSize(width, height)
	return width[0], height[0]
end

-- Show or hide the cursor.
-- @param show boolean True to show the cursor, false to hide it.
function System.showCursor(show)
	lib.microSystemShowCursor(show)
end

-- Focus the window.
function System.focusWindow()
	lib.microSystemFocusWindow()
end

-- Check if a gamepad is connected.
function System.isGamepadConnected()
	return lib.microSystemIsGamepadConnected()
end

return System
