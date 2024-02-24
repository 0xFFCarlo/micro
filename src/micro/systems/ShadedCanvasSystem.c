#include "ShadedCanvasSystem.h"
#include "../components/RenderingComponents.h"
#include "../core/ECS.h"
#include "../core/Graphics.h"
#include <stdlib.h>

void shadedCanvasSystem(float dt)
{
  (void)(dt); // Unused parameter

  CShadedCanvas *components_shaded_canvas = (CShadedCanvas *)
    microECSComponentsGet(cid_shadedCanvas);
  const unsigned int
    components_count = microECSComponentsCount(cid_shadedCanvas);

  // Store old view and set canvas view full window
  int old_shader = microShaderGetCurrent();
  MicroView old_view = microViewGet();

  // Update all shaded canvases
  for (unsigned int i = 0; i < components_count; i++)
  {
    CShadedCanvas *scanvas = &components_shaded_canvas[i];
    if (scanvas->needsUpdate == 0)
      continue;
    microGraphicsRenderToCanvas(scanvas->canvasId);
    microGraphicsClear(0, 0, 0, 0);
    microShaderApply(scanvas->shaderId);
    microViewSet((MicroView){.viewportX = 0,
                             .viewportY = 0,
                             .viewportWidth = scanvas->width,
                             .viewportHeight = scanvas->height,
                             .centerX = scanvas->width / 2.0,
                             .centerY = scanvas->height / 2.0,
                             .width = scanvas->width,
                             .height = scanvas->height,
                             .rotation = 0,
                             .flipY = 1});
    microViewApply();
    int textureId = microCanvasGetTextureId(scanvas->canvasId);
    int texWidth, texHeight;
    microTextureGetSize(textureId, &texWidth, &texHeight);
    microGraphicsDrawRect(textureId, 0, 0, texWidth, texHeight, 0, 0,
                          scanvas->width, scanvas->height, 1.0, 1.0, 1.0, 1.0);
    microGraphicsDisplay();
    scanvas->needsUpdate = 0;
  }

  // Restore old view and shader
  microGraphicsRenderToScreen();
  microShaderApply(old_shader);
  microViewSet(old_view);
  microViewApply();
}
