#pragma once

#include <SDL2/SDL.h>
#include <imgui.h>

#include <ovis/core/scene_controller.hpp>

namespace ovis {

class ImGuiEndFrameController : public SceneController {
 public:
  static inline constexpr std::string_view Name() { return "ImGuiEndFrame"; }

  ImGuiEndFrameController();

  void Update(std::chrono::microseconds delta_time) override;

 private:
  std::array<SDL_Cursor*, ImGuiMouseCursor_COUNT> cursors_;
};

}  // namespace ovis
