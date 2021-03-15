#include "toolbar.hpp"

#include "../editor_window.hpp"
#include "asset_editors/asset_editor.hpp"
#include "asset_viewer_window.hpp"
#include "inspector_window.hpp"
#include "log_window.hpp"

#include <imgui_internal.h>

#include <ovis/core/platform.hpp>
#include <ovis/engine/input.hpp>

namespace ovis {
namespace editor {

Toolbar::Toolbar() : UiWindow("Toolbar", "") {
  SetStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  SetStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  SetStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  SetFlags(ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

  icons_.save = LoadTexture2D("icon-save", EditorWindow::instance()->context());
  icons_.undo = LoadTexture2D("icon-undo", EditorWindow::instance()->context());
  icons_.redo = LoadTexture2D("icon-redo", EditorWindow::instance()->context());
  icons_.package = LoadTexture2D("icon-package", EditorWindow::instance()->context());
  icons_.windows = LoadTexture2D("icon-windows", EditorWindow::instance()->context());

  SubscribeToEvent(KeyPressEvent::TYPE);
}

void Toolbar::ProcessEvent(Event* event) {
  if (event->type() == KeyPressEvent::TYPE) {
    auto* key_press_event = static_cast<KeyPressEvent*>(event);
    const bool ctrl_command_pressed =
        GetPlatform() == Platform::MACOS
            ? input()->GetKeyState(Key::META_LEFT) || input()->GetKeyState(Key::META_RIGHT)
            : input()->GetKeyState(Key::CONTROL_LEFT) || input()->GetKeyState(Key::CONTROL_RIGHT);
    if (ctrl_command_pressed && key_press_event->key() == Key::KEY_S) {
      Save();
      event->StopPropagation();
    }
  }
}

void Toolbar::BeforeBegin() {
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
  ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, icon_size_.y + 10));
  ImGui::SetNextWindowViewport(viewport->ID);
}

void Toolbar::DrawContent() {
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(48, 48, 48)));

  if (ImGui::ImageButton(icons_.save.get(), icon_size_)) {
    Save();
  }
  if (ImGui::IsItemHovered()) {
    if (AssetEditor::last_focused_document_window != nullptr) {
      ImGui::SetTooltip("Save: %s", AssetEditor::last_focused_document_window->asset_id().c_str());
    } else {
      ImGui::SetTooltip("Nothing to save");
    }
  }

  ImGui::SameLine();
  if (ImGui::ImageButton(icons_.undo.get(), icon_size_)) {
    Undo();
  }
  if (ImGui::IsItemHovered()) {
    if (AssetEditor::last_focused_document_window != nullptr && AssetEditor::last_focused_document_window->CanUndo()) {
      ImGui::SetTooltip("Undo");
    } else {
      ImGui::SetTooltip("Nothing to undo");
    }
  }

  ImGui::SameLine();
  if (ImGui::ImageButton(icons_.redo.get(), icon_size_)) {
    Redo();
  }
  if (ImGui::IsItemHovered()) {
    if (AssetEditor::last_focused_document_window != nullptr && AssetEditor::last_focused_document_window->CanRedo()) {
      ImGui::SetTooltip("Redo");
    } else {
      ImGui::SetTooltip("Nothing to redo");
    }
  }

  ImGui::SameLine();
  if (ImGui::ImageButton(icons_.package.get(), icon_size_)) {
    scene()->AddController("PackagingWindow");
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Package game");
  }

  ImGui::SameLine();
  ImVec2 window_button_pos = ImGui::GetCursorPos();
  if (ImGui::ImageButton(icons_.windows.get(), icon_size_)) {
    ImGui::OpenPopup("testtest");
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Windows");
  }
  ImVec2 next_line_pos = ImGui::GetCursorPos();
  ImGui::SetNextWindowPos({window_button_pos.x, next_line_pos.y});
  if (ImGui::BeginPopup("testtest")) {
    if (ImGui::Selectable("Log")) {
      if (auto log_window = scene()->GetController<LogWindow>("Log"); log_window != nullptr) {
        log_window->Remove();
      } else {
        scene()->AddController<LogWindow>();
      }
    }
    if (ImGui::Selectable("Assets")) {
      if (auto assets_window = scene()->GetController<AssetViewerWindow>("Assets"); assets_window != nullptr) {
        assets_window->Remove();
      } else {
        scene()->AddController<AssetViewerWindow>();
      }
    }
    if (ImGui::Selectable("Inspector")) {
      if (auto inspector_window = scene()->GetController<InspectorWindow>("Inspector"); inspector_window != nullptr) {
        inspector_window->Remove();
      } else {
        scene()->AddController<InspectorWindow>();
      }
    }
    ImGui::EndPopup();
  }

  ImGui::PopStyleColor();
}

void Toolbar::Save() {
  if (AssetEditor::last_focused_document_window != nullptr) {
    AssetEditor::last_focused_document_window->Save();
  }
}

void Toolbar::Undo() {
  if (AssetEditor::last_focused_document_window != nullptr) {
    AssetEditor::last_focused_document_window->Undo();
  }
}

void Toolbar::Redo() {
  if (AssetEditor::last_focused_document_window != nullptr) {
    AssetEditor::last_focused_document_window->Redo();
  }
}

}  // namespace editor
}  // namespace ovis
