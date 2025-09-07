#ifndef RENDERING_SYSTEM_H
#define RENDERING_SYSTEM_H

void microRenderingSysSetLayerSetupCb(int layerId,
                                      void (*setup_layer)(int layerId));

#endif /* end of include guard: RENDERING_SYSTEM_H */
