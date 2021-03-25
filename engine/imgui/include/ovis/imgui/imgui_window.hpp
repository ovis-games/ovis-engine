#pragma once

#include <tuple>
#include <variant>
#include <vector>

#include <imgui.h>

#include <ovis/core/scene_controller.hpp>

namespace ovis {

class ImGuiWindow : public SceneController {
 public:
  ImGuiWindow(const std::string& id, const std::string& window_title = "");

  void Update(std::chrono::microseconds delta_time) override;

  inline void Focus() { should_focus_ = true; }
  inline bool has_focus() const { return has_focus_; }

  inline ImGuiID dockspace_id() const { return dockspace_id_; }
  inline void SetDockSpaceId(ImGuiID dockspace_id) {
    dockspace_id_ = dockspace_id;
    dock_next_frame_ = true;
  }

  using ImGuiStyleValue = std::variant<float, ImVec2>;
  void SetStyleVar(ImGuiStyleVar style_id, ImGuiStyleValue value);
  void SetFlags(ImGuiWindowFlags window_flags);

 protected:
  bool HasFrameStarted() const;
  virtual void BeforeBegin() {}
  virtual void DrawContent() {}

  std::string imgui_id_;
  ImGuiWindowFlags window_flags_;
  bool should_focus_ = false;
  bool has_focus_ = false;

  ImGuiID dockspace_id_ = 0;
  bool dock_next_frame_ = false;

  std::vector<std::tuple<ImGuiStyleVar, ImGuiStyleValue>> style_vars_;
};

}  // namespace ovis
