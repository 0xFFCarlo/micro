#include "Pathfinding.h"
#include "../util/debug.h"
#include <math.h>
#include <memory.h>
#include <stdio.h>

#define MAX_STARTS_POINTS 128
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct microFloodCell
{
  unsigned int distance;
  unsigned int iteration;
  unsigned char is_start;
} microFloodCell;

typedef struct microFloodPoint
{
  unsigned int x;
  unsigned int y;
} microFloodPoint;

struct microFloodFinder
{
  unsigned int map_width;
  unsigned int map_height;
  unsigned int *solid_map;
  microFloodCell *cells;
  microFloodPoint starts[MAX_STARTS_POINTS];
  unsigned int current_iteration;
  unsigned int start_count;
  unsigned int found_starts;
  microFloodPoint *queue;
  int queue_pos;
  int queue_size;
  unsigned int target_x;
  unsigned int target_y;
};

microFloodFinder *microFloodFinderCreate(int map_width, int map_height,
                                         unsigned int *solid_map)
{
  microFloodFinder *finder = malloc(sizeof(microFloodFinder));
  finder->map_width = map_width;
  finder->map_height = map_height;
  finder->solid_map = solid_map;
  finder->cells = malloc(sizeof(microFloodCell) * map_width * map_height);
  memset(finder->cells, 0, sizeof(microFloodCell) * map_width * map_height);
  finder->queue = malloc(sizeof(microFloodPoint) * map_width * map_height);
  finder->queue_pos = 0;
  finder->current_iteration = 1;
  finder->start_count = 0;
  finder->found_starts = 0;
  finder->target_x = -1;
  finder->target_y = -1;
  finder->queue_pos = 0;
  finder->queue_size = 1;

  for (int y = 0; y < map_height; y++)
    for (int x = 0; x < map_width; x++)
      if (solid_map[y * map_width + x])
        finder->cells[y * map_width + x].distance = INT32_MAX;

  return finder;
}

// Add a start point
void microFloodFinderAddStart(microFloodFinder *finder, unsigned int tx,
                              unsigned int ty)
{
  if (finder->start_count < MAX_STARTS_POINTS)
  {
    finder->starts[finder->start_count].x = tx;
    finder->starts[finder->start_count].y = ty;
    finder->cells[ty * finder->map_width + tx].is_start = 1;
    finder->start_count++;
  }
  else
  {
    debug_print("Too many start points");
  }
}

// Remove a start point
void microFloodFinderRemoveStart(microFloodFinder *finder, unsigned int tx,
                                 unsigned int ty)
{
  for (unsigned int i = 0; i < finder->start_count; i++)
  {
    if (finder->starts[i].x == tx && finder->starts[i].y == ty)
    {
      finder->cells[ty * finder->map_width + tx].is_start = 0;
      finder->start_count--;
      finder->starts[i] = finder->starts[finder->start_count];
      return;
    }
  }
  debug_print("Start point not found");
}

// Clear start points
void microFloodFinderClearStarts(microFloodFinder *finder)
{
  while (finder->start_count > 0)
  {
    finder->start_count--;
    finder
      ->cells[finder->starts[finder->start_count].y * finder->map_width +
              finder->starts[finder->start_count].x]
      .is_start = 0;
  }
}

// Set the target point
void microFloodFinderSetTarget(microFloodFinder *finder, unsigned int tx,
                               unsigned int ty)
{
  assert(tx >= 0 && tx < finder->map_width);
  assert(ty >= 0 && ty < finder->map_height);
  finder->target_x = tx;
  finder->target_y = ty;
}

static inline unsigned int cell_distance(microFloodFinder *finder, int x, int y)
{
  if (x >= (int)finder->map_width || y >= (int)finder->map_height || x < 0 ||
      y < 0)
    return INT32_MAX;
  return finder->cells[y * finder->map_width + x].distance;
}

static inline int cell_is_solid(microFloodFinder *finder, unsigned int x,
                                unsigned int y)
{
  assert(x >= 0 && x < finder->map_width);
  assert(y >= 0 && y < finder->map_height);
  return finder->solid_map[y * finder->map_width + x];
}

static inline unsigned int cell_get_iteration(microFloodFinder *finder,
                                              unsigned int x, unsigned int y)
{
  assert(x >= 0 && x < finder->map_width);
  assert(y >= 0 && y < finder->map_height);
  return finder->cells[y * finder->map_width + x].iteration;
}

static inline int cell_is_start(microFloodFinder *finder, unsigned int x,
                                unsigned int y)
{
  assert(x >= 0 && x < finder->map_width);
  assert(y >= 0 && y < finder->map_height);
  return finder->cells[y * finder->map_width + x].is_start;
}

int microFloodFinderGetDistance(microFloodFinder *finder, unsigned int tx,
                                unsigned int ty)
{
  return finder->cells[ty * finder->map_width + tx].distance;
}

// Compute the paths from the start points to the targets
// TODO: no need to clear whole map each time, we can use a max
// value from last update as threshold
void microFloodFinderUpdate(microFloodFinder *finder)
{
  assert(finder->target_x >= 0 && finder->target_x < finder->map_width);
  assert(finder->target_y >= 0 && finder->target_y < finder->map_height);

  finder->queue[0].x = finder->target_x;
  finder->queue[0].y = finder->target_y;
  finder->queue_pos = 0;
  finder->queue_size = 1;
  finder->cells[finder->target_y * finder->map_width + finder->target_x]
    .distance = 0;
  finder->found_starts = 0;
  microFloodPoint *q = finder->queue;

  // Flood until eached all starts points or all cells are done
  while (finder->found_starts < finder->start_count &&
         finder->queue_pos < finder->queue_size)
  {
    microFloodPoint curr = q[finder->queue_pos];

    // Set distance
    microFloodCell *cell = &finder->cells[curr.y * finder->map_width + curr.x];
    const unsigned int dist = cell->distance;

    if (cell_is_start(finder, curr.x, curr.y))
      finder->found_starts++;

    // Add neighbors to queue that has not been visited and are not solid
    if (curr.x > 0 &&
        cell_get_iteration(finder, curr.x - 1, curr.y) !=
          finder->current_iteration &&
        !cell_is_solid(finder, curr.x - 1, curr.y))
    {
      q[finder->queue_size].x = curr.x - 1;
      q[finder->queue_size].y = curr.y;
      finder->queue_size++;
      finder->cells[(curr.y) * finder->map_width + (curr.x - 1)]
        .iteration = finder->current_iteration;
      finder->cells[(curr.y) * finder->map_width + (curr.x - 1)]
        .distance = dist + 1;
    }
    if (curr.x < finder->map_width - 1 &&
        cell_get_iteration(finder, curr.x + 1, curr.y) !=
          finder->current_iteration &&
        !cell_is_solid(finder, curr.x + 1, curr.y))
    {
      q[finder->queue_size].x = curr.x + 1;
      q[finder->queue_size].y = curr.y;
      finder->queue_size++;
      finder->cells[(curr.y) * finder->map_width + (curr.x + 1)]
        .iteration = finder->current_iteration;
      finder->cells[(curr.y) * finder->map_width + (curr.x + 1)]
        .distance = dist + 1;
    }
    if (curr.y > 0 &&
        cell_get_iteration(finder, curr.x, curr.y - 1) !=
          finder->current_iteration &&
        !cell_is_solid(finder, curr.x, curr.y - 1))
    {
      q[finder->queue_size].x = curr.x;
      q[finder->queue_size].y = curr.y - 1;
      finder->queue_size++;
      finder->cells[(curr.y - 1) * finder->map_width + (curr.x)]
        .iteration = finder->current_iteration;
      finder->cells[(curr.y - 1) * finder->map_width + (curr.x)]
        .distance = dist + 1;
    }
    if (curr.y < finder->map_height - 1 &&
        cell_get_iteration(finder, curr.x, curr.y + 1) !=
          finder->current_iteration &&
        !cell_is_solid(finder, curr.x, curr.y + 1))
    {
      q[finder->queue_size].x = curr.x;
      q[finder->queue_size].y = curr.y + 1;
      finder->queue_size++;
      finder->cells[(curr.y + 1) * finder->map_width + (curr.x)]
        .iteration = finder->current_iteration;
      finder->cells[(curr.y + 1) * finder->map_width + (curr.x)]
        .distance = dist + 1;
    }

    // Next cell
    finder->queue_pos++;
  }

  finder->current_iteration++;
}

MicroDirection microFloodFinderGetDir(microFloodFinder *finder, unsigned int tx,
                                      unsigned int ty)
{
  unsigned int tmp_dist;
  unsigned int curr_dist = cell_distance(finder, tx, ty);

  tmp_dist = cell_distance(finder, tx - 1, ty);
  if (tmp_dist < curr_dist)
    return DIR_LEFT;
  tmp_dist = cell_distance(finder, tx + 1, ty);
  if (tmp_dist < curr_dist)
    return DIR_RIGHT;
  tmp_dist = cell_distance(finder, tx, ty - 1);
  if (tmp_dist < curr_dist)
    return DIR_UP;
  tmp_dist = cell_distance(finder, tx, ty + 1);
  if (tmp_dist < curr_dist)
    return DIR_DOWN;

  return DIR_NONE;
}

// Save distances as csv
void microFloodFinderSaveMap(microFloodFinder *finder, const char *filename)
{
  FILE *file = fopen(filename, "w");
  for (unsigned int y = 0; y < finder->map_height; y++)
  {
    for (unsigned int x = 0; x < finder->map_width; x++)
    {
      MicroDirection dir = microFloodFinderGetDir(finder, x, y);
      if (dir == DIR_LEFT)
        fprintf(file, "<");
      else if (dir == DIR_RIGHT)
        fprintf(file, ">");
      else if (dir == DIR_UP)
        fprintf(file, "^");
      else if (dir == DIR_DOWN)
        fprintf(file, "v");
      else
        fprintf(file, "O");
      if (x < finder->map_width - 1)
        fprintf(file, ",");
    }
    fprintf(file, "\n");
  }
  fclose(file);
}

// Free memory
void microFloodFinderDestroy(microFloodFinder *finder)
{
  assert(finder != NULL);
  assert(finder->cells != NULL);
  assert(finder->queue != NULL);
  free(finder->cells);
  free(finder->queue);
  free(finder);
}
