#pragma once

#include <imgui.h>

#include <ovis/engine/scene_controller.hpp>

namespace ovis {

class ImGuiSceneController : public SceneController {
 public:
  ImGuiSceneController(ImGuiContext* context);

  void DrawImGui() override;
  void ProcessEvent(Event* event) override;

 private:
  ImGuiContext* context_;

  // Indicates whether a mouse button was just pressed
  bool mouse_button_pressed_[5] = {false, false, false, false, false};
  bool mouse_button_down_[5] = {false, false, false, false, false};
};

}  // namespace ovis
