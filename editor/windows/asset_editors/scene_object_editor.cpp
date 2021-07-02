#include "scene_object_editor.hpp"

#include "../../editor_window.hpp"
#include "../../imgui_extensions/input_asset.hpp"
#include "../../imgui_extensions/input_serializable.hpp"
#include "../../imgui_extensions/texture_button.hpp"
#include "../../imgui_extensions/splitter.hpp"

#include <imgui.h>
#include <imgui_internal.h>

#include <ovis/core/asset_library.hpp>

namespace ovis {
namespace editor {

SceneObjectEditor::SceneObjectEditor(const std::string& scene_object_asset)
    : SceneViewEditor(scene_object_asset, false) {
  object_ = game_scene()->CreateObject(scene_object_asset);
  SelectObject(object_.get());
  SetupJsonFile(object_->Serialize());
}

void SceneObjectEditor::DrawContent() {
  DrawToolbar();
  
  float viewport_height = 0.0f;
  float animation_pane_height = 0.0f;
  ImGui::HorizontalSplitter("ContentSplitter", &viewport_height, &animation_pane_height);

  if (ImGui::BeginChild("Viewport", ImVec2(0, viewport_height))) {
    DrawViewport();
  }
  ImGui::EndChild();

  if (ImGui::BeginChild("AnimationPane", ImVec2(0, animation_pane_height))) {
    DrawAnimationPane();
  }
  ImGui::EndChild();
}

void SceneObjectEditor::Save() {
  SaveFile("json", object_->Serialize().dump());
}

void SceneObjectEditor::SubmitChanges() {
  UpdateSceneEditingCopy();
  SubmitJsonFile(object_->Serialize());
}

void SceneObjectEditor::JsonFileChanged(const json& data, const std::string& file_type) {
  SDL_assert(file_type == "json");
  object_->Deserialize(data);
  UpdateSceneEditingCopy();
}

void SceneObjectEditor::DrawObjectTree() {
  DrawObjectHierarchy(object_.get());
}

void SceneObjectEditor::DrawAnimationPane() {
  ImGui::Text("Animation");
  if (ImGui::BeginChild("AnimationPane", ImVec2(0, 0), ImGuiWindowFlags_HorizontalScrollbar)) {
    DrawAnimationPaneObjectHierarchy(object_.get());
  }
  ImGui::EndChild();
}

void SceneObjectEditor::DrawAnimationPaneObjectHierarchy(SceneObject* object) {
  if (object == nullptr) {
    return;
  }

  auto* window = ImGui::GetCurrentWindow();

  auto name = object->name();
  if (ImGui::TreeNodeBehavior(window->GetID(name.begin(), name.end()), 0, name.begin(), name.end())) {
    for (const auto& child : object->children()) {
      DrawAnimationPaneObjectHierarchy(child.get());
    }
    for (const auto& component : object->GetComponentIds()) {
      if (ImGui::TreeNodeEx(component.c_str(), ImGuiTreeNodeFlags_Leaf)) {
        ImGui::TreePop();
      }
    }
    ImGui::TreePop();
  }
}

}  // namespace editor
}  // namespace ovis
