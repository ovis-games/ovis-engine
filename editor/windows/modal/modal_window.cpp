#include "modal_window.hpp"

#include <imgui.h>

#include <ovis/utils/log.hpp>

namespace ovis {
namespace editor {

ModalWindow::ModalWindow(const std::string& id, const std::string& window_title) : ImGuiWindow(id, window_title) {}

void ModalWindow::Update(std::chrono::microseconds delta_time) {
  if (!HasFrameStarted()) {
    return;
  }

  ImGui::OpenPopup(imgui_id_.c_str());
  if (ImGui::BeginPopupModal(imgui_id_.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    DrawContent();
    ImGui::EndPopup();
  }
}

}  // namespace editor
}  // namespace ovis
