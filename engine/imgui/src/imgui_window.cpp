#include <ovis/utils/log.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/imgui/imgui_end_frame_controller.hpp>
#include <ovis/imgui/imgui_start_frame_controller.hpp>
#include <ovis/imgui/imgui_window.hpp>

namespace ovis {

ImGuiWindow::ImGuiWindow(const std::string& id, const std::string& window_title)
    : SceneController(id), imgui_id_(window_title.length() > 0 ? window_title + "##" + id : id), window_flags_(0) {
  UpdateAfter<ImGuiStartFrameController>();
  UpdateBefore<ImGuiEndFrameController>();
}

void ImGuiWindow::Update(std::chrono::microseconds delta_time) {
  if (!HasFrameStarted()) {
    return;
  }

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
    Close();
  }
}

void ImGuiWindow::SetStyleVar(ImGuiStyleVar style_id, ImGuiStyleValue value) {
  style_vars_.push_back(std::make_tuple(style_id, value));
}

bool ImGuiWindow::HasFrameStarted() const {
  auto start_frame_controller = scene()->GetController<ImGuiStartFrameController>();
  return start_frame_controller != nullptr && start_frame_controller->frame_started_;
}

}  // namespace ovis
