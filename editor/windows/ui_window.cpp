#include "ui_window.hpp"

#include <ovis/core/log.hpp>

namespace ove {

UiWindow::UiWindow(const std::string& id, const std::string& window_title, ImGuiWindowFlags window_flags)
    : ovis::SceneController(id),
      imgui_id_(window_title.length() > 0 ? window_title + "##" + id : id),
      window_flags_(window_flags) {
  UpdateAfter("EditorWindowController");
}

void UiWindow::DrawImGui() {
  if (dock_next_frame_) {
    ImGui::SetNextWindowDockID(dockspace_id_);
    dock_next_frame_ = false;
  }
  
  if (should_focus_) {
    ImGui::SetNextWindowFocus();
    should_focus_ = false;
  }

  for (const auto& style_var : style_vars_) {
    const auto& value = std::get<1>(style_var);
    if (std::holds_alternative<float>(value)) {
      ImGui::PushStyleVar(std::get<0>(style_var), std::get<float>(value));
    } else {
      ImGui::PushStyleVar(std::get<0>(style_var), std::get<ImVec2>(value));
    }
  }
  BeforeBegin();
  bool keep_open = true;
  if (ImGui::Begin(imgui_id_.c_str(), &keep_open, window_flags_)) {
    if (style_vars_.size() > 0) {
      ImGui::PopStyleVar(style_vars_.size());
    }
    has_focus_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    DrawContent();
  }
  ImGui::End();

  if (!keep_open) {
    Remove();
  }
}

void UiWindow::SetStyleVar(ImGuiStyleVar style_id, ImGuiStyleValue value) {
  style_vars_.push_back(std::make_tuple(style_id, value));
}

}  // namespace ove
