#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "../util/Types.h"

typedef struct microFloodFinder microFloodFinder;

// Create a new microFloodFinder object from map size and map data
// The map data is 0 for empty, 1 for obstacles
microFloodFinder *microFloodFinderCreate(int map_width, int map_height,
                                         unsigned int *solid_map);

// Add a start point
void microFloodFinderAddStart(microFloodFinder *finder, unsigned int tx,
                              unsigned int ty);

// Remove a start point
void microFloodFinderRemoveStart(microFloodFinder *finder, unsigned int tx,
                                 unsigned int ty);

// Clear start points
void microFloodFinderClearStarts(microFloodFinder *finder);

// Set the target point
void microFloodFinderSetTarget(microFloodFinder *finder, unsigned int tx,
                               unsigned int ty);

// Compute the paths from the start points to the targets
void microFloodFinderUpdate(microFloodFinder *finder);

// Get the direction to move to reach the target
MicroDirection microFloodFinderGetDir(microFloodFinder *finder, unsigned int tx,
                                      unsigned int ty);

// Free memory
void microFloodFinderDestroy(microFloodFinder *finder);

int microFloodFinderGetDistance(microFloodFinder *finder, unsigned int tx,
                                unsigned int ty);

void microFloodFinderSaveMap(microFloodFinder *finder, const char *filename);

#endif // PATHFINDING_H
