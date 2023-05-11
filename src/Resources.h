#ifndef RESOURCES_H
#define RESOURCES_H

#define MICRO_TYPE_TEXTURE 0
#define MICRO_TYPE_SOUND 1
#define MICRO_TYPE_MUSIC 2

extern int microResourceLoad(const char* name, const char* filepath, const char* type);
extern int microResourceGet(const char* name);
extern int microResourceFree(const char* name);
extern void microResourceFreeAll();

#endif /* end of include guard: RESOURCES_H */

