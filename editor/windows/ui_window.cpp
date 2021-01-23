#include "ui_window.hpp"
#include <ovis/core/log.hpp>

namespace ove {

UiWindow::UiWindow(const std::string& id, const std::string& window_title, ImGuiWindowFlags window_flags)
    : ovis::SceneController(id),
      imgui_id_(window_title.length() > 0 ? window_title + "##" + id : id),
      window_flags_(window_flags) {
  UpdateAfter("EditorWindowController");}

void UiWindow::DrawImGui() {
  if (dock_next_frame_) {
    ImGui::SetNextWindowDockID(dockspace_id_);
  }
  bool keep_open = true;
  if (ImGui::Begin(imgui_id_.c_str(), &keep_open, window_flags_)) {
    has_focus_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    DrawContent();
  }
  ImGui::End();

  if (!keep_open) {
    Remove();
  }
}

}  // namespace ove
