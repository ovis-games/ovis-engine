#pragma once

#include <string_view>

#include <imgui.h>

#include <ovis/core/scene_controller.hpp>

namespace ovis {

class ImGuiStartFrameController : public SceneController {
 friend class ImGuiEndFrameController;
 friend class ImGuiRenderPass;

 public:
  static inline constexpr std::string_view Name() { return "ImGuiStartFrame"; }

  ImGuiStartFrameController();

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

 private:
  std::unique_ptr<ImGuiContext, void (*)(ImGuiContext*)> imgui_context_;

  // Indicates whether a mouse button was just pressed
  bool mouse_button_pressed_[5] = {false, false, false, false, false};
};

}  // namespace ovis
