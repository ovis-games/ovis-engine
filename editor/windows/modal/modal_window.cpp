#include "modal_window.hpp"

#include <imgui.h>

#include <ovis/core/log.hpp>

namespace ove {

ModalWindow::ModalWindow(const std::string& id, const std::string& window_title, ImGuiWindowFlags window_flags)
    : UiWindow(id, window_title, window_flags) {}

void ModalWindow::DrawImGui() {
  ImGui::OpenPopup(imgui_id_.c_str());
  if (ImGui::BeginPopupModal(imgui_id_.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    DrawContent();
    ImGui::EndPopup();
  }
}

}  // namespace ove
