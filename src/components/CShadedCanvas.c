#include <stdlib.h>
#include "CShadedCanvas.h"
#include "../ECS.h"

int cid_shadedCanvas;

void RegisterCShadedCanvas()
{
  cid_shadedCanvas = microECSComponentRegister(sizeof(CShadedCanvas), NULL);
}
