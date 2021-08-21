#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>

#include <ovis/utils/class.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/graphics/graphics_context.hpp>
#include <ovis/rendering/rendering_viewport.hpp>

namespace ovis {

class GraphicsDevice;
class ProfilingLog;

struct WindowDescription {
  std::string title;
  int width = 1280;
  int height = 720;
};

class Window : public RenderingViewport {
  MAKE_NON_COPY_OR_MOVABLE(Window);

 public:
  Window(const WindowDescription& description);
  ~Window();

  inline static const std::vector<Window*>& all_windows() { return all_windows_; }
  inline static Window* GetWindowById(Uint32 id) {
    for (auto window : all_windows()) {
      if (window->id() == id) {
        return window;
      }
    }
    return nullptr;
  }
  inline SDL_Window* sdl_window() const { return sdl_window_; }
  inline Uint32 id() const { return id_; }
  inline bool is_open() const { return is_open_; }

  RenderTargetConfiguration* GetDefaultRenderTargetConfiguration() override;

  Vector2 GetDimensions() const override;
  void Resize(int width, int height);

  virtual bool SendEvent(const SDL_Event& event);
  virtual void Update(std::chrono::microseconds delta_time);
  void Render() override;

 private:
  static std::vector<Window*> all_windows_;
  SDL_Window* sdl_window_;
  Uint32 id_;
  bool is_open_ = true;

  GraphicsContext graphics_context_;
  Scene scene_;
};

}  // namespace ovis
