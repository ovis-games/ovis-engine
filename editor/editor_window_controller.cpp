#include "editor_window_controller.hpp"

#include "editor_asset_library.hpp"
#include "editor_window.hpp"
#include "windows/asset_editors/asset_editor.hpp"
#include "windows/asset_importers/asset_importer.hpp"
#include "windows/log_window.hpp"
#include "windows/asset_viewer_window.hpp"
#include "windows/inspector_window.hpp"

#include <emscripten/html5.h>
#include <imgui_internal.h>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/log.hpp>
#include <ovis/engine/engine.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/window.hpp>

namespace ove {

EditorWindowController::EditorWindowController() : ovis::SceneController("EditorWindowController") {
  ovis::CreateApplicationAssetLibrary<EditorAssetLibrary>("/assets/");

  icons_.save = ovis::LoadTexture2D("icon-save", EditorWindow::instance()->context());
  icons_.undo = ovis::LoadTexture2D("icon-undo", EditorWindow::instance()->context());
  icons_.redo = ovis::LoadTexture2D("icon-redo", EditorWindow::instance()->context());
  icons_.package = ovis::LoadTexture2D("icon-package", EditorWindow::instance()->context());
  icons_.windows = ovis::LoadTexture2D("icon-windows", EditorWindow::instance()->context());
}

void EditorWindowController::Update(std::chrono::microseconds delta_time) {
  if (AssetEditor::last_focused_document_window != nullptr) {
    AssetEditor::last_focused_document_window->Update(delta_time);
  }
}

void EditorWindowController::DrawImGui() {
  DrawToolbar();
  DrawDockSpace();
}

bool EditorWindowController::ProcessEvent(const SDL_Event& event) {
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_s && (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)) != 0) {
      Save();
      return true;
    } else if (event.key.keysym.sym == SDLK_z && (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)) != 0 &&
               (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0) {
      Redo();
      return true;
    } else if (event.key.keysym.sym == SDLK_z && (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)) != 0) {
      Undo();
      return true;
    }
  } else if (event.type == SDL_DROPFILE) {
    const std::string filename = event.drop.file;
    SDL_free(event.drop.file);
    ImportAsset(filename);
    return true;
  }

  if (AssetEditor::last_focused_document_window) {
    return AssetEditor::last_focused_document_window->ProcessEvent(event);
  } else {
    return false;
  }
}

void EditorWindowController::DrawDockSpace() {
}

void EditorWindowController::DrawToolbar() {
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
  ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, icon_size_.y + 10));
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                  ImGuiWindowFlags_NoSavedSettings;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Toolbar", nullptr, window_flags);
  ImGui::PopStyleVar(3);

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
    std::string undo_description = "Nothing to undo";
    if (AssetEditor::last_focused_document_window != nullptr &&
        AssetEditor::last_focused_document_window->GetActionHistory()->undo_possible()) {
      undo_description = AssetEditor::last_focused_document_window->GetActionHistory()->undo_description();
    }
    ImGui::SetTooltip("Undo: %s", undo_description.c_str());
  }

  ImGui::SameLine();
  if (ImGui::ImageButton(icons_.redo.get(), icon_size_)) {
    Redo();
  }
  if (ImGui::IsItemHovered()) {
    std::string redo_description = "Nothing to redo";
    if (AssetEditor::last_focused_document_window != nullptr &&
        AssetEditor::last_focused_document_window->GetActionHistory()->redo_possible()) {
      redo_description = AssetEditor::last_focused_document_window->GetActionHistory()->redo_description();
    }
    ImGui::SetTooltip("Redo: %s", redo_description.c_str());
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

  ImGui::End();
}

void EditorWindowController::Save() {
  if (AssetEditor::last_focused_document_window != nullptr) {
    AssetEditor::last_focused_document_window->Save();
  }
}

void EditorWindowController::Undo() {
  if (AssetEditor::last_focused_document_window != nullptr) {
    AssetEditor::last_focused_document_window->GetActionHistory()->Undo();
  }
}

void EditorWindowController::Redo() {
  if (AssetEditor::last_focused_document_window != nullptr) {
    AssetEditor::last_focused_document_window->GetActionHistory()->Redo();
  }
}

}  // namespace ove