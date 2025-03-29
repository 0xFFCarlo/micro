local ffi = require("ffi")
local lib = require("src/lua/libmicro")

ffi.cdef([[
    // Loads a resource from a file and stores it in memory.
    int microResourceLoad(const char *name, const char *filepath, const char *type);
    // Automatically loads resources from a directory.
    int microResourceAutoLoad(const char *dirpath);
    // Loads a font resource from a file.
    int microResourceLoadFont(const char *name, const char *filepath, unsigned int font_size, unsigned int filter);
    // Gets a resource id by name.
    int microResourceGet(const char *name);
    // Frees a resource from memory.
    int microResourceFree(const char *name);
    // Frees all resources from memory.
    void microResourceFreeAll();
]])

---@class ResourcesModule
local Resources = {}

--- Loads a resource from a file and stores it in memory.
--- @param name string The name of the resource.
--- @param filepath string The path to the file.
--- @param type string The type of the resource ("texture", "sound", "music").
--- @return number resource id on success, or -1 on error.
function Resources.load(name, filepath, type)
	return lib.microResourceLoad(name, filepath, type)
end

--- Automatically loads resources (textures, sounds, music, etc.) from a directory.
--- @param dirpath string The path to the directory.
--- @return number 1 on success, -1 on error.
function Resources.autoLoad(dirpath)
	return lib.microResourceAutoLoad(dirpath)
end

--- Loads a font resource from a file.
--- @param name string The name of the resource.
--- @param filepath string The path to the file.
--- @param font_size number The size of the font.
--- @param filter number The filter to use for the font (e.g., MICRO_FILTER_NEAREST or MICRO_FILTER_LINEAR).
--- @return number resource id on success, or -1 on error.
function Resources.loadFont(name, filepath, font_size, filter)
	return lib.microResourceLoadFont(name, filepath, font_size, filter)
end

--- Gets a resource id by name.
--- @param name string The name of the resource.
--- @return number resource id on success, or -1 if not found.
function Resources.get(name)
	return lib.microResourceGet(name)
end

--- Frees a resource from memory.
--- @param name string The name of the resource.
--- @return number 1 on success, or -1 if not found.
function Resources.free(name)
	return lib.microResourceFree(name)
end

--- Frees all resources from memory.
function Resources.freeAll()
	lib.microResourceFreeAll()
end

return Resources
