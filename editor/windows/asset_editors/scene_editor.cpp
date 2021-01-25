#include "scene_editor.hpp"

#include "../../editor_window.hpp"
#include "../../imgui_extensions/input_asset.hpp"
#include "../../imgui_extensions/input_serializable.hpp"

#include <ovis/core/asset_library.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/scene_controller.hpp>
#include <ovis/engine/scene_object.hpp>

namespace ove {

namespace {

void CreateObject(ovis::Scene* scene, const ovis::json& data) {
  SDL_assert(data.is_string());
  SDL_assert(!scene->ContainsObject(data.get<std::string>()));
  scene->CreateObject(data.get<std::string>());
}

void UndoCreateObject(ovis::Scene* scene, const ovis::json& data) {
  SDL_assert(data.is_string());
  SDL_assert(scene->ContainsObject(data.get<std::string>()));
  scene->DeleteObject(data.get<std::string>());
}

void DeleteObject(ovis::Scene* scene, const ovis::json& data) {
  SDL_assert(data.contains("name"));
  SDL_assert(data["name"].is_string());
  SDL_assert(data.contains("objects"));
  SDL_assert(data["objects"].is_object());
  SDL_assert(scene->ContainsObject(data["name"]));
  SDL_assert(scene->GetObject(data["name"])->Serialize() == data["object"]);
  scene->DeleteObject(data["name"]);
}

void UndoDeleteObject(ovis::Scene* scene, const ovis::json& data) {
  SDL_assert(data.contains("name"));
  SDL_assert(data["name"].is_string());
  SDL_assert(data.contains("objects"));
  SDL_assert(data["objects"].is_object());
  SDL_assert(!scene->ContainsObject(data["name"]));
  scene->CreateObject(data["name"], data["objects"]);
}

void RenameObject(ovis::Scene* scene, const ovis::json& data) {
  SDL_assert(data.contains("old_name") && data["old_name"].is_string());
  SDL_assert(data.contains("new_name") && data["new_name"].is_string());
  SDL_assert(scene->ContainsObject(data["old_name"].get<std::string>()));
  SDL_assert(!scene->ContainsObject(data["new_name"].get<std::string>()));

  const std::string old_name = data["old_name"];
  const std::string new_name = data["new_name"];

  const auto serialized_object = scene->GetObject(old_name)->Serialize();
  scene->DeleteObject(old_name);

  scene->CreateObject(new_name, serialized_object);
}

void UndoRenameObject(ovis::Scene* scene, const ovis::json& data) {
  SDL_assert(data.contains("old_name") && data["old_name"].is_string());
  SDL_assert(data.contains("new_name") && data["new_name"].is_string());
  SDL_assert(!scene->ContainsObject(data["old_name"].get<std::string>()));
  SDL_assert(scene->ContainsObject(data["new_name"].get<std::string>()));

  const std::string old_name = data["old_name"];
  const std::string new_name = data["new_name"];

  const auto serialized_object = scene->GetObject(new_name)->Serialize();
  scene->DeleteObject(new_name);

  scene->CreateObject(old_name, serialized_object);
}

void AddComponent(ovis::Scene* scene, const ovis::json& data) {
  SDL_assert(data.contains("object_name"));
  SDL_assert(data["object_name"].is_string());
  SDL_assert(data.contains("component_id"));
  SDL_assert(data["component_id"].is_string());
  SDL_assert(scene->ContainsObject(data["object_name"]));
  SDL_assert(!scene->GetObject(data["object_name"])->HasComponent(data["component_id"]));
  scene->GetObject(data["object_name"])->AddComponent(data["component_id"]);
}

void UndoAddComponent(ovis::Scene* scene, const ovis::json& data) {
  SDL_assert(data.contains("object_name"));
  SDL_assert(data["object_name"].is_string());
  SDL_assert(data.contains("component_id"));
  SDL_assert(data["component_id"].is_string());
  SDL_assert(scene->ContainsObject(data["object_name"]));
  SDL_assert(scene->GetObject(data["object_name"])->HasComponent(data["component_id"]));
  scene->GetObject(data["object_name"])->RemoveComponent(data["component_id"]);
}

void ChangeComponent(ovis::Scene* scene, const ovis::json& data) {
  SDL_assert(data.contains("object_name"));
  SDL_assert(data["object_name"].is_string());
  SDL_assert(data.contains("component_id"));
  SDL_assert(data["component_id"].is_string());
  SDL_assert(data.contains("before"));
  SDL_assert(data.contains("after"));
  SDL_assert(scene->ContainsObject(data["object_name"]));
  SDL_assert(scene->GetObject(data["object_name"])->HasComponent(data["component_id"]));
  scene->GetObject(data["object_name"])->GetComponent(data["component_id"])->Deserialize(data["after"]);
}

void UndoChangeComponent(ovis::Scene* scene, const ovis::json& data) {
  SDL_assert(data.contains("object_name"));
  SDL_assert(data["object_name"].is_string());
  SDL_assert(data.contains("component_id"));
  SDL_assert(data["component_id"].is_string());
  SDL_assert(data.contains("before"));
  SDL_assert(data.contains("after"));
  SDL_assert(scene->ContainsObject(data["object_name"]));
  SDL_assert(scene->GetObject(data["object_name"])->HasComponent(data["component_id"]));
  scene->GetObject(data["object_name"])->GetComponent(data["component_id"])->Deserialize(data["before"]);
}

}  // namespace

SceneEditor::SceneEditor(const std::string& scene_asset) : AssetEditor(scene_asset), action_history_(&scene_) {
  action_history_.RegisterAction("CreateObject", &CreateObject, &UndoCreateObject);
  action_history_.RegisterAction("DeleteObject", &DeleteObject, &UndoDeleteObject);
  action_history_.RegisterAction("RenameObject", &RenameObject, &UndoRenameObject);
  action_history_.RegisterAction("AddComponent", &AddComponent, &UndoAddComponent);
  action_history_.RegisterAction("ChangeComponent", &ChangeComponent, &UndoChangeComponent);

  const std::optional<std::string> serialized_scene = LoadTextFile("json");
  if (serialized_scene) {
    try {
      scene_.Deserialize(ovis::json::parse(*serialized_scene));
    } catch (const ovis::json::exception& exception) {
      ovis::LogE("Failed to parse scene file: {}", exception.what());
    }
  } else {
    ovis::LogE("Could not load scene asset: {}", asset_id());
  }

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
  DrawObjectList();
  DrawObjectComponentList();
}

// void SceneEditor::DrawPropertyWindows() {
//   DrawObjectList();
//   DrawObjectComponentList();
//   DrawSceneProperties();
// }

void SceneEditor::Save() {
  SaveFile("json", scene_.Serialize().dump());
}

void SceneEditor::DrawObjectList() {
  ImVec2 window_size = ImGui::GetWindowSize();
  ImGui::BeginChild("ObjectList", ImVec2(0, window_size.y / 2 - ImGui::GetFrameHeightWithSpacing()), true);
  for (ovis::SceneObject* object : scene_.GetObjects(true)) {
    bool item_selected = selected_ == object->name();
    if (item_selected && is_renaming_) {
      char buffer[512];
      strcpy(buffer, object->name().c_str());
      if (ImGui::InputText(("###" + object->name()).c_str(), buffer, 512,
                           ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (!scene_.ContainsObject(buffer)) {
          is_renaming_ = false;
          action_history_.Do("RenameObject", {{"old_name", object->name()}, {"new_name", buffer}},
                             "Rename object {} to {}", object->name(), buffer);
          selected_ = buffer;
        } else {
          // Object already exists or old and new names are the same
          ovis::LogI("The object '{}' does already exist", buffer);
        }
      }

      if (ImGui::IsItemDeactivated()) {
        is_renaming_ = false;
      }
      ImGui::SetKeyboardFocusHere(0);
    } else {
      if (ImGui::Selectable(object->name().c_str(), &item_selected)) {
        selected_ = object->name();
      }
      if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        is_renaming_ = true;
      }
    }
  }
  ImGui::EndChild();
  // ImGui::BeginChild("SceneDocumentButtons");

  // if (ImGui::Button("Add")) {
  //   std::string object_name = "NewObject";

  //   for (int i = 2; scene_.ContainsObject(object_name); object_name = "NewObject" + std::to_string(i), ++i)
  //     ;

  //   action_history_.Do("CreateObject", object_name, "Create object {}", object_name);
  //   selected_ = object_name;
  //   is_renaming_ = true;
  // }
  // ImGui::SameLine();

  // if (ImGui::Button("Remove")) {
  //   if (scene_.ContainsObject(selected_)) {
  //     action_history_.Do("ChangeObject", {{"before", scene_.GetObject(selected_)->Serialize()}}, "Delete object {}",
  //                        selected_);
  //     selected_ = "";
  //     is_renaming_ = false;
  //   }
  // }
  // ImGui::EndChild();
}

void SceneEditor::DrawObjectComponentList() {
  ImVec2 window_size = ImGui::GetWindowSize();
  if (ImGui::BeginChild("Object Properties", ImVec2(0, window_size.y / 2 - ImGui::GetFrameHeightWithSpacing()))) {
    ovis::SceneObject* selected_object = scene_.GetObject(selected_);
    if (selected_object != nullptr) {
      ImGui::Text("%s", selected_object->name().c_str());
      ImGui::SameLine();
      if (ImGui::BeginCombo("##AddComponent", "Add Component", ImGuiComboFlags_NoArrowButton)) {
        for (const auto& component_id : ovis::SceneObjectComponent::GetRegisteredComponents()) {
          if (!selected_object->HasComponent(component_id)) {
            if (ImGui::Selectable(component_id.c_str())) {
              action_history_.Do("AddComponent",
                                 {{"object_name", selected_object->name()}, {"component_id", component_id}},
                                 "Add component {} to {}", component_id, selected_object->name());
            }
          }
        }
        ImGui::EndCombo();
      }

      ImGui::BeginChild("Components", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
      for (const auto& component_id : selected_object->GetComponentIds()) {
        ovis::Serializable* component = selected_object->GetComponent(component_id);

        const auto before = component->Serialize();  // TODO: this is aweful
        if (ImGui::InputSerializable(component_id.c_str(), component)) {
          const auto after = component->Serialize();
          action_history_.Do("ChangeComponent",
                             {{"object_name", selected_object->name()},
                              {"component_id", component_id},
                              {"before", before},
                              {"after", after}},
                             "Change property in component {} of {}", component_id, selected_object->name());
        }
      }
      ImGui::EndChild();

      ImGui::BeginChild("Scene Object Buttons");

      ImGui::EndChild();
    } else {
      ImGui::Text("Select an object in the scene to display its components.");
    }
  }
  ImGui::EndChild();
}  // namespace ove

void SceneEditor::DrawSceneProperties() {
  if (ImGui::Begin("Scene Properties")) {
    if (ImGui::CollapsingHeader("Controllers", ImGuiTreeNodeFlags_DefaultOpen)) {
      for (const auto& controller : ovis::GetApplicationAssetLibrary()->GetAssetsWithType("scene_controller")) {
        bool has_controller = scene_.GetController(controller) != nullptr;
        if (ImGui::Checkbox(controller.c_str(), &has_controller)) {
          if (has_controller) {
            scene_.AddController(controller);
          } else {
            scene_.RemoveController(controller);
          }
        }
      }
    }
  }
  ImGui::End();
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

}  // namespace ove