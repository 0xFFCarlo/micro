

#include "ShadedCanvasSystem.h"
#include "../Graphics.h"
#include "../components/RenderingComponents.h"
#include "../ECS.h"
#include <stdlib.h>

int shaded_canvas_system_query = -1;

void shadedCanvasSystem(float dt) {

  if (shaded_canvas_system_query == -1) {
    int components[1] = {cid_shadedCanvas};
    shaded_canvas_system_query = microECSQueryCreate(components, 1, NULL);
  }
  
  ecs_entity_list entities = microECSQueryRun(shaded_canvas_system_query);
  if (entities.size == 0) return;
  
  //Store old view and set canvas view full window
  int old_shader = microShaderGetCurrent();
  MicroView old_view = microViewGet();
  
  //Update all shaded canvases
  for (int i = 0; i < entities.size; i++) {
    const int entityId = entities.entityIds[i];
    CShadedCanvas* scanvas = (CShadedCanvas*)microECSEntityGetComponent(entityId, cid_shadedCanvas);
    microGraphicsRenderToCanvas(scanvas->canvasId);
    microGraphicsClear(0, 0, 0, 0);
    microShaderApply(scanvas->shaderId);
    microViewSet((MicroView){
        .viewportX = 0,
        .viewportY = 0,
        .viewportWidth = scanvas->width,
        .viewportHeight = scanvas->height,
        .centerX = scanvas->width/2.0,
        .centerY = scanvas->height/2.0,
        .width = scanvas->width,
        .height = scanvas->height,
        .rotation = 0,
        .flipY = 1
        });
    microViewApply();
    int textureId = microCanvasGetTextureId(scanvas->canvasId);
    int texWidth, texHeight;
    microTextureGetSize(textureId, &texWidth, &texHeight);
    microGraphicsDrawRect(textureId, 0, 0, texWidth, texHeight, 0, 0, scanvas->width, scanvas->height, 1.0, 1.0, 1.0, 1.0);
    microGraphicsDisplay();
  }
  
  //Restore old view and shader
  microGraphicsRenderToScreen();
  microShaderApply(old_shader);
  microViewSet(old_view);
  microViewApply();
}
