#ifndef RESOURCES_H
#define RESOURCES_H

extern int microResourceLoad(const char* name, const char* filepath, const char* type);
extern int microResourceGet(const char* name);
extern int microResourceFree(const char* name);
extern void microResourceFreeAll();

#endif /* end of include guard: RESOURCES_H */

