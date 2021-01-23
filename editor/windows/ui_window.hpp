#pragma once

#include <imgui.h>

#include <ovis/engine/scene_controller.hpp>

namespace ove {

class UiWindow : public ovis::SceneController {
 public:
  UiWindow(const std::string& id, const std::string& window_title = "", ImGuiWindowFlags window_flags = 0);

  void DrawImGui() override;

  inline void Focus() { should_focus_ = true; }
  inline bool has_focus() const { return has_focus_;  }

  inline ImGuiID dockspace_id() const { return dockspace_id_; }
  inline void SetDockSpaceId(ImGuiID dockspace_id) {
    dockspace_id_ = dockspace_id;
    dock_next_frame_ = true;
  }

 protected:
  virtual void DrawContent() {}

  std::string imgui_id_;
  ImGuiWindowFlags window_flags_;
  bool should_focus_ = false;
  bool has_focus_ = false;

  ImGuiID dockspace_id_ = 0;
  bool dock_next_frame_ = false;
};

}  // namespace ove
