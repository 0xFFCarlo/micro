local ffi = require("ffi")

local libName
if ffi.os == "OSX" then
	libName = "libmicro.dylib"
elseif ffi.os == "Windows" then
	libName = "micro.dll"
else
	libName = "libmicro.so"
end

local function script_dir()
	local src = debug.getinfo(1, "S").source
	if type(src) == "string" and src:sub(1, 1) == "@" then
		local full = src:sub(2)
		-- Strip trailing filename, keep directory.
		return full:match("^(.*[/\\])") or "./"
	end
	return "./"
end

local baseDir = script_dir()
local localLibPath = baseDir .. "../lib/" .. libName

local ok, lib = pcall(ffi.load, localLibPath)
if not ok then
	-- Fallback to standard dynamic loader search path for global installs.
	lib = ffi.load(libName)
end

return lib
