#ifndef RESOURCES_H
#define RESOURCES_H

/**
 * @brief Loads a resource from a file and stores it in memory.
 * @param name The name of the resource
 * @param filepath The path to the file
 * @param type The type of the resource ("texture", "sound", "music")
 * @return resource id or -1 on error
 */
extern int microResourceLoad(const char *name, const char *filepath,
                             const char *type);

/**
 * @brief Gets a resource id by name
 * @param name The name of the resource
 * @return resource id or -1 if not found
 */
extern int microResourceGet(const char *name);

/**
 * @brief Frees a resource from memory
 * @param name The name of the resource
 * @return 1 on success, -1 if not found
 */
extern int microResourceFree(const char *name);

/**
 * @brief Frees all resources from memory
 */
extern void microResourceFreeAll();

#endif /* end of include guard: RESOURCES_H */
