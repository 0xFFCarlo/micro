local ffi = require("ffi")

local libName
if ffi.os == "OSX" then
	libName = "libmicro.dylib"
elseif ffi.os == "Windows" then
	libName = "micro.dll"
else
	libName = "libmicro.so"
end

local libPath = "micro/lib/" .. libName
local lib = ffi.load(libPath)

return lib
