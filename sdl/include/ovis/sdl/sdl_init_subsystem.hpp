#pragma once

#include "SDL.h"

#include "ovis/utils/class.hpp"

namespace ovis {

// This is a helper class that ensures a specific subsystem is initialized RAII style.
template <Uint32 SUBSYSTEM>
class SDLInitSubsystem {
  MAKE_NON_COPY_OR_MOVABLE(SDLInitSubsystem);

 public:
  SDLInitSubsystem() { SDL_InitSubSystem(SUBSYSTEM); }
  ~SDLInitSubsystem() { SDL_QuitSubSystem(SUBSYSTEM); }
};

}  // namespace ovis
