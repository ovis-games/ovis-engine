#pragma once

#include <ovis/core/scene_controller.hpp>

namespace ovis
{

class ImGuiEndFrameController : public SceneController {
 public:
  static inline constexpr std::string_view Name() { return "ImGuiStartFrame"; }

  ImGuiEndFrameController();

  void Update(std::chrono::microseconds delta_time) override;

 private:

};

} // namespace ovis
