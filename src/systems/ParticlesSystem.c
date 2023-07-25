#include "ParticlesSystem.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void particlesSystem(float dt)
{
  CParticleEmitter *components_emitter = (CParticleEmitter *)
    microECSComponentsGet(cid_particle_emitter);
  const unsigned int
    components_count = microECSComponentsCount(cid_particle_emitter);

  // Update emitters position to be bound to entity
  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_particle_emitter, i);
    CPosition *position = (CPosition *)microECSEntityGetComponent(entityId,
                                                                  cid_position);
    CParticleEmitter *emitter = &components_emitter[i];
    microParticleEmitterSetPosition(emitter->emitterId,
                                    position->x + emitter->offsetX,
                                    position->y + emitter->offsetY);
  }

  // Update particles
  microParticleEmittersUpdate(dt);
}
