#pragma once

#include "ovis/core/application.hpp"
#include "ovis/sdl/sdl_init_subsystem.hpp"

namespace ovis {

class SDLEventProcessor : public ApplicationJob, public SDLInitSubsystem<SDL_INIT_EVENTS | SDL_INIT_VIDEO> {
  public:
   SDLEventProcessor() : ApplicationJob("SDLEventProcessor") {}
   Result<> Prepare(const Nothing&) override { return Success; }
   Result<> Execute(const double& delta_time) override;
};

}  // namespace ovis
