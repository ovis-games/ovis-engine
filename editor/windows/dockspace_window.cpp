#include "dockspace_window.hpp"

#include <imgui_internal.h>

namespace ove {

DockspaceWindow::DockspaceWindow()
    : UiWindow("Dockspace Window", "",
               ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoNavFocus) {
  SetStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  SetStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  SetStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
}

void DockspaceWindow::BeforeBegin() {
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  // TODO: get actual toolbar icon size
  ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + 28 + 10));
  ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - 28 - 10));
  ImGui::SetNextWindowViewport(viewport->ID);
}

void DockspaceWindow::DrawContent() {
  ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
  if (ImGui::DockBuilderGetNode(dockspace_id) == NULL) {
    dockspace_main_ = dockspace_id;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockBuilderRemoveNode(dockspace_main_);  // Clear out existing layout
    ImGui::DockBuilderAddNode(dockspace_main_,
                              ImGuiDockNodeFlags_DockSpace);  // Add empty node
    ImGui::DockBuilderSetNodeSize(dockspace_main_, viewport->Size);

    dockspace_left_ = ImGui::DockBuilderSplitNode(dockspace_main_, ImGuiDir_Left, 0.2f, NULL, &dockspace_main_);
    dockspace_right_ = ImGui::DockBuilderSplitNode(dockspace_main_, ImGuiDir_Right, 1.0f / 4.0f, NULL, &dockspace_main_);
    dockspace_bottom_ = ImGui::DockBuilderSplitNode(dockspace_main_, ImGuiDir_Down, 0.25f, NULL, &dockspace_main_);

    ImGui::DockBuilderDockWindow("Inspector", dockspace_right_);
    ImGui::DockBuilderDockWindow("Log", dockspace_bottom_);
    ImGui::DockBuilderDockWindow("Assets", dockspace_left_);

    ImGui::DockBuilderFinish(dockspace_main_);
  }

  ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, 0xff000000);
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), 0);
  ImGui::PopStyleColor();
}

}  // namespace ove
