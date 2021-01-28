#include "scene_editor.hpp"

#include "../../editor_window.hpp"
#include "../../imgui_extensions/input_asset.hpp"
#include "../../imgui_extensions/input_serializable.hpp"

#include <ovis/core/asset_library.hpp>
#include <ovis/core/utils.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/scene_controller.hpp>
#include <ovis/engine/scene_object.hpp>
#include <imgui_stdlib.h>

namespace ove {

const SceneEditor::SelectedObject SceneEditor::SelectedObject::NONE = {""};

SceneEditor::SceneEditor(const std::string& scene_asset) : AssetEditor(scene_asset) {
  SetupJsonFile(scene_.Serialize());
  CreateSceneViewport();
  scene_viewport_->AddRenderPass("SpriteRenderer");
  scene_viewport_->SetScene(&scene_);

  ovis::Lua::on_error.Subscribe([this](const std::string&) {
    if (state_ == State::RUNNING) {
      state_ = State::PAUSED;
    }
  });
}

void SceneEditor::Update(std::chrono::microseconds delta_time) {
  if (state_ == State::RUNNING) {
    scene_.Update(delta_time);
  }
}

bool SceneEditor::ProcessEvent(const SDL_Event& event) {
  if (scene_window_focused_) {
    return scene_.ProcessEvent(event);
  } else {
    return false;
  }
}

void SceneEditor::DrawContent() {
  scene_viewport_->Render(false);
  if (state_ == State::RUNNING) {
    if (ImGui::Button("Pause")) {
      state_ = State::PAUSED;
    }
  } else {
    if (ImGui::Button("Play")) {
      if (state_ == State::STOPPED) {
        serialized_scene_ = scene_.Serialize();
        // Reload so scripts are upt to date
        scene_.Deserialize(serialized_scene_);
        scene_.Play();
      }
      state_ = State::RUNNING;
    }
  }
  if (ImGui::Button("Stop")) {
    if (state_ != State::STOPPED) {
      scene_.Stop();
      scene_.Deserialize(serialized_scene_);
    }
    state_ = State::STOPPED;
  }

  ImVec4 border_color(0, 0, 0, 0);
  if (state_ == State::RUNNING) {
    border_color = ImGui::GetStyle().Colors[ImGuiCol_NavHighlight];
  }
  ImGui::Image(scene_viewport_->color_texture()->texture(), {512, 512}, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1),
               border_color);
  scene_window_focused_ = ImGui::IsWindowFocused();
}

void SceneEditor::DrawInspectorContent() {
  bool scene_changed = false;
  if (DrawObjectList()) {
    scene_changed = true;
  }
  if (DrawObjectComponentList()) {
    scene_changed = true;
  }
  if (scene_changed) {
    serialized_scene_ = scene_.Serialize();
    SubmitJsonFile(serialized_scene_);
  }
}

void SceneEditor::Save() {
  SaveFile("json", serialized_scene_.dump());
}

bool SceneEditor::DrawObjectList() {
  bool scene_changed = false;

  ImVec2 window_size = ImGui::GetWindowSize();
  ImGui::BeginChild("ObjectView", ImVec2(0, window_size.y / 2), true);

  ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                       ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

  ImGuiTreeNodeFlags scene_node_flags = tree_node_flags | ImGuiTreeNodeFlags_AllowItemOverlap;
  if (std::holds_alternative<SelectedScene>(selection_)) {
    scene_node_flags |= ImGuiTreeNodeFlags_Selected;
  }
  if (ImGui::TreeNodeEx(asset_id().c_str(), scene_node_flags)) {
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      selection_ = SelectedScene{};
    }

    const std::string selected_object_name =
        ovis::get_with_default<SelectedObject>(selection_, SelectedObject::NONE).name;
    scene_.GetObjects(&cached_scene_objects_, true);
    for (ovis::SceneObject* object : cached_scene_objects_) {
      SDL_assert(object != nullptr);

      ImGuiTreeNodeFlags scene_object_flags = tree_node_flags | ImGuiTreeNodeFlags_Leaf;
      if (object->name() == selected_object_name) {
        scene_object_flags |= ImGuiTreeNodeFlags_Selected;
      }

      if (renaming_state_ == RenamingState::IS_NOT_RENAMING || object->name() != selected_object_name) {
        if (ImGui::TreeNodeEx(object->name().c_str(), scene_object_flags)) {
          if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            selection_ = SelectedObject{object->name()};
            // is_renaming_selected_object_ = false;
          }
          if (ImGui::BeginPopupContextItem()) {
            if (ImGui::Selectable("Rename")) {
              renaming_state_ = RenamingState::STARTED_RENAMING;
            }
            if (ImGui::Selectable("Remove")) {
              scene_.DeleteObject(object->name());
              scene_changed = true;
            }
            ImGui::EndPopup();
          }
          ImGui::TreePop();
        }
      } else {
        std::string new_object_name = object->name();
        ImGui::TreePush(object->name().c_str());
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("Renaming", &new_object_name, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
          if (new_object_name != object->name()) {
            // TODO: check for validity: alphanumeric+underscore+space
            if (scene_.ContainsObject(new_object_name)) {
              // TODO: Show message box
            } else {
              scene_.CreateObject(new_object_name, object->Serialize());
              scene_.DeleteObject(object->name());
              scene_changed = true;
            }
          }
          renaming_state_ = RenamingState::IS_NOT_RENAMING;
        }
        if (renaming_state_ == RenamingState::STARTED_RENAMING) {
          ImGui::SetKeyboardFocusHere();
          renaming_state_ = RenamingState::IS_RENAMING;
        } else if (!ImGui::IsItemActive()) {
          renaming_state_ = RenamingState::IS_NOT_RENAMING;
        }
        ImGui::PopItemWidth();
        ImGui::TreePop();
      }
    }

    ImGui::TreePop();
  }
  ImGui::EndChild();

  return scene_changed;
}

bool SceneEditor::DrawObjectComponentList() {
  bool object_changed = false;

  ImVec2 window_size = ImGui::GetWindowSize();
  if (ImGui::BeginChild("Object Properties", ImVec2(0, window_size.y / 2 - ImGui::GetFrameHeightWithSpacing()))) {
    const std::string selected_object_name =
        ovis::get_with_default<SelectedObject>(selection_, SelectedObject::NONE).name;

    if (selected_object_name.size() != 0) {
      ovis::SceneObject* selected_object = scene_.GetObject(selected_object_name);

      ImGui::Text("%s", selected_object->name().c_str());
      ImGui::SameLine();
      if (ImGui::BeginCombo("##AddComponent", "Add Component", ImGuiComboFlags_NoArrowButton)) {
        for (const auto& component_id : ovis::SceneObjectComponent::GetRegisteredComponents()) {
          if (!selected_object->HasComponent(component_id)) {
            if (ImGui::Selectable(component_id.c_str())) {
              selected_object->AddComponent(component_id);
              object_changed = true;
            }
          }
        }
        ImGui::EndCombo();
      }

      ImGui::BeginChild("Components", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
      // TODO: iterate over json object and not the serialize the components on every frame
      for (const auto& component_id : selected_object->GetComponentIds()) {
        ovis::Serializable* component = selected_object->GetComponent(component_id);
        if (ImGui::InputSerializable(component_id.c_str(), component)) {
          object_changed = true;
        }
      }
      ImGui::EndChild();

      ImGui::BeginChild("Scene Object Buttons");

      ImGui::EndChild();
    } else {
      if (ImGui::InputJson("Scene", &serialized_scene_, *scene_.GetSchema(), ImGuiInputJsonFlags_IgnoreEnclosingObject)) {
        // TODO: move to appropriate function and use serialized scene and do not re-serialze each frame
        scene_.Deserialize(serialized_scene_);
        object_changed = true;
      }
    }
  }
  ImGui::EndChild();

  return object_changed;
}

void SceneEditor::CreateSceneViewport() {
  ovis::RenderTargetViewportDescription scene_viewport_description;
  scene_viewport_description.color_description.texture_description.width = 512;
  scene_viewport_description.color_description.texture_description.height = 512;
  scene_viewport_description.color_description.texture_description.format = ovis::TextureFormat::RGBA_UINT8;
  scene_viewport_description.color_description.texture_description.filter = ovis::TextureFilter::POINT;
  scene_viewport_description.color_description.texture_description.mip_map_count = 0;
  scene_viewport_ = std::make_unique<ovis::RenderTargetViewport>(
      EditorWindow::instance()->context(), EditorWindow::instance()->resource_manager(), scene_viewport_description);
}

void SceneEditor::JsonFileChanged(const ovis::json& data, const std::string& file_type) {
  if (file_type == "json") {
    serialized_scene_ = data;
    scene_.Deserialize(serialized_scene_);
  }
}

}  // namespace ove