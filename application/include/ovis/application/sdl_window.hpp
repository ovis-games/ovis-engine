#pragma once

#include <chrono>
#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "SDL.h"

#include "ovis/application/tick_receiver.hpp"
#include "ovis/utils/all.hpp"
#include "ovis/utils/class.hpp"
#include "ovis/core/scene.hpp"
#include "ovis/graphics/graphics_context.hpp"

namespace ovis {

class GraphicsDevice;
class ProfilingLog;

struct SDLWindowDescription {
  std::string title = "Ovis Window";
  int width = 1280;
  int height = 720;
  Scene* scene = nullptr;
};

// This is a helper class that ensures a specific subsystem is initialized on construction and uninitalized on
// destruction.
template <Uint32 SUBSYSTEM>
class SDLInitSubsystem {
  MAKE_NON_COPY_OR_MOVABLE(SDLInitSubsystem);

 public:
  SDLInitSubsystem() { SDL_InitSubSystem(SUBSYSTEM); }
  ~SDLInitSubsystem() { SDL_QuitSubSystem(SUBSYSTEM); }
};

class SDLWindow : public TickReceiver, public SDLInitSubsystem<SDL_INIT_VIDEO> {
  MAKE_NON_COPY_OR_MOVABLE(SDLWindow);

 public:
  SDLWindow(const SDLWindowDescription& description);
  ~SDLWindow();

  inline SDL_Window* sdl_window() const { return sdl_window_; }
  inline Uint32 id() const { return id_; }
  inline bool is_open() const { return is_open_; }

  Vector2 GetDrawableSize() const;
  void Resize(int width, int height);

  virtual bool SendEvent(const SDL_Event& event);
  void Update(std::chrono::microseconds delta_time) override;

 private:
  SDL_Window* sdl_window_;
  Uint32 id_;
  bool is_open_ = true;

  SDL_GLContext opengl_context_;
  GraphicsContext graphics_context_;
};

}  // namespace ovis
