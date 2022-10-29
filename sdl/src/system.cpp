#include "ovis/sdl/system.hpp"

#include "SDL.h"
#include "SDL_video.h"
#include "ovis/sdl/sdl_init_subsystem.hpp"

namespace ovis {

std::vector<Display> GetDisplays() {
  SDLInitSubsystem<SDL_INIT_VIDEO> video_system_initialization;

  const auto display_count = SDL_GetNumVideoDisplays();
  std::vector<Display> displays(display_count);

  for (int i = 0; i < display_count; ++i) {
    displays[i].name = SDL_GetDisplayName(i);
    SDL_GetDisplayDPI(i, &displays[i].diagonal_dpi, &displays[i].horizontal_dpi, &displays[i].vertical_dpi);

    SDL_Rect bounds;
    SDL_GetDisplayBounds(i, &bounds);
    displays[i].bounds.width = bounds.w;
    displays[i].bounds.height = bounds.h;
    displays[i].bounds.left = bounds.x;
    displays[i].bounds.top = bounds.y;
  }

  return displays;
}

}  // namespace ovis
