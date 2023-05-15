#ifndef CSHADED_CANVAS_H
#define CSHADED_CANVAS_H

typedef struct {
  int width, height;
  int shaderId;
  int canvasId;
} CShadedCanvas;

extern int cid_shadedCanvas;
extern void RegisterCShadedCanvas();




#endif // CSHADED_CANVAS_H
