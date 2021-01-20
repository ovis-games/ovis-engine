#include "modal_window.hpp"

#include <imgui.h>

#include <ovis/core/log.hpp>

namespace ove {

ModalWindow::ModalWindow(const std::string& controller_name, const std::string& window_title)
    : ovis::SceneController(controller_name), window_name_(window_title + "##" + controller_name) {}

void ModalWindow::DrawImGui() {
  ImGui::OpenPopup(window_name_.c_str());
  if (ImGui::BeginPopupModal(window_name_.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    DrawContent();
    ImGui::EndPopup();
  }
}

}  // namespace ove
