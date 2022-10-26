#pragma once

#include <chrono>
#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "SDL.h"

#include "ovis/utils/all.hpp"
#include "ovis/utils/class.hpp"
#include "ovis/core/application.hpp"
#include "ovis/core/scene.hpp"
#include "ovis/graphics/graphics_context.hpp"
#include "ovis/sdl/sdl_init_subsystem.hpp"

namespace ovis {

class GraphicsDevice;
class ProfilingLog;

struct SDLWindowDescription {
  std::string title = "Ovis Window";
  int width = 1280;
  int height = 720;
  Scene* scene = nullptr;
};

class SDLWindow : public All<SDLWindow>, public SDLInitSubsystem<SDL_INIT_VIDEO> {
  MAKE_NON_COPY_OR_MOVABLE(SDLWindow);

 public:
  SDLWindow(const SDLWindowDescription& description);
  ~SDLWindow();

  inline SDL_Window* sdl_window() const { return sdl_window_; }
  inline Uint32 id() const { return id_; }
  inline bool is_open() const { return is_open_; }

  void Close();

  Vector2 GetDrawableSize() const;
  void Resize(int width, int height);

  void ProcessEvent(const SDL_Event& event);

  // Result<> Prepare(const Nothing&) override { return Success; }
  // Result<> Execute(const double& delta_time) override { return Success; }

  static SDLWindow* GetWindowById(Uint32 id);

 private:
  SDL_Window* sdl_window_;
  Uint32 id_;
  bool is_open_ = true;

  SDL_GLContext opengl_context_;
  GraphicsContext graphics_context_;
};

}  // namespace ovis
