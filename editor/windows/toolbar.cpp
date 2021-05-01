#include "toolbar.hpp"

#include "../editor_window.hpp"
#include "../imgui_extensions/texture_button.hpp"
#include "asset_editors/asset_editor.hpp"
#include "asset_viewer_window.hpp"
#include "inspector_window.hpp"
#include "log_window.hpp"

#include <imgui_internal.h>

#include <ovis/utils/platform.hpp>
#include <ovis/input/key.hpp>
#include <ovis/input/key_events.hpp>
#include <ovis/imgui/imgui_start_frame_controller.hpp>

namespace ovis {
namespace editor {

Toolbar::Toolbar() : ImGuiWindow("Toolbar", "") {
  SetStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  SetStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  SetStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  SetFlags(ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

  SubscribeToEvent(KeyPressEvent::TYPE);
}

void Toolbar::ProcessEvent(Event* event) {
  if (event->type() == KeyPressEvent::TYPE) {
    auto* key_press_event = static_cast<KeyPressEvent*>(event);
    const bool ctrl_command_pressed = GetPlatform() == Platform::MACOS
                                          ? IsKeyPressed(Key::MetaLeft()) || IsKeyPressed(Key::MetaRight())
                                          : IsKeyPressed(Key::ControlLeft()) || IsKeyPressed(Key::ControlRight());
    if (ctrl_command_pressed && key_press_event->key() == Key::S()) {
      Save();
      event->StopPropagation();
    }
  }
}

void Toolbar::BeforeBegin() {
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
  ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 28 + 10)); // y: icon size + padding
  ImGui::SetNextWindowViewport(viewport->ID);
}

void Toolbar::DrawContent() {
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(48, 48, 48)));

  ImFont* font_awesome = scene()->GetController<ImGuiStartFrameController>()->GetFont("FontAwesomeSolid");

  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf0c7")) {
    Save();
  }
  ImGui::PopFont();
  if (ImGui::IsItemHovered()) {
    if (AssetEditor::last_focused_document_window != nullptr) {
      ImGui::SetTooltip("Save: %s", AssetEditor::last_focused_document_window->asset_id().c_str());
    } else {
      ImGui::SetTooltip("Nothing to save");
    }
  }

  ImGui::SameLine();
  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf0e2")) {
    Undo();
  }
  ImGui::PopFont();
  if (ImGui::IsItemHovered()) {
    if (AssetEditor::last_focused_document_window != nullptr && AssetEditor::last_focused_document_window->CanUndo()) {
      ImGui::SetTooltip("Undo");
    } else {
      ImGui::SetTooltip("Nothing to undo");
    }
  }

  ImGui::SameLine();
  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf01e")) {
    Redo();
  }
  ImGui::PopFont();
  if (ImGui::IsItemHovered()) {
    if (AssetEditor::last_focused_document_window != nullptr && AssetEditor::last_focused_document_window->CanRedo()) {
      ImGui::SetTooltip("Redo");
    } else {
      ImGui::SetTooltip("Nothing to redo");
    }
  }

  ImGui::SameLine();
  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf466")) {
    scene()->AddController("PackagingWindow");
  }
  ImGui::PopFont();
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Package game");
  }

  ImGui::SameLine();
  ImVec2 window_button_pos = ImGui::GetCursorPos();
  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf2d2")) {
    ImGui::OpenPopup("testtest");
  }
  ImGui::PopFont();
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
