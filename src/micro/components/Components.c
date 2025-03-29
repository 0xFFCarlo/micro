#include "Components.h"

int microComponentsRegisterAll()
{
  RegisterLogicComponents();
  RegisterMotionComponents();
  RegisterRenderingComponents();
  RegisterAudioComponents();
  return 0;
}
