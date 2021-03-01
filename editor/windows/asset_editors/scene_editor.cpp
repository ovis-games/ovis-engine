#include "scene_editor.hpp"

#include "../../editor_window.hpp"
#include "../../imgui_extensions/input_asset.hpp"
#include "../../imgui_extensions/input_serializable.hpp"
#include "../../imgui_extensions/texture_button.hpp"

#include <imgui_stdlib.h>
#include <ovis/base/transform_component.hpp>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/utils.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/scene_controller.hpp>
#include <ovis/engine/scene_object.hpp>
#include <ovis/rendering2d/sprite_component.hpp>

namespace ovis {
namespace editor {

const SceneEditor::SelectedObject SceneEditor::SelectedObject::NONE = {""};

SceneEditor::SceneEditor(const std::string& scene_asset) : AssetEditor(scene_asset) {
  SetupJsonFile(scene_.Serialize());

  Lua::on_error.Subscribe([this](const std::string&) {
    if (state_ == State::RUNNING) {
      state_ = State::PAUSED;
    }
  });

  icons_.play = LoadTexture2D("icon-play", EditorWindow::instance()->context());
  icons_.pause = LoadTexture2D("icon-pause", EditorWindow::instance()->context());
  icons_.stop = LoadTexture2D("icon-stop", EditorWindow::instance()->context());
  icons_.move = LoadTexture2D("icon-move", EditorWindow::instance()->context());
  icons_.rotate = LoadTexture2D("icon-rotate", EditorWindow::instance()->context());
  icons_.scale = LoadTexture2D("icon-scale", EditorWindow::instance()->context());

  camera_.SetProjectionType(ProjectionType::ORTHOGRAPHIC);
  camera_.SetNearClipPlane(0.0f);
  camera_.SetFarClipPlane(1000.0f);
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
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(48, 48, 48)));
  if (state_ == State::RUNNING) {
    if (ImGui::TextureButton(icons_.pause.get())) {
      state_ = State::PAUSED;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Pause");
    }
  } else {
    if (ImGui::TextureButton(icons_.play.get())) {
      if (state_ == State::STOPPED) {
        serialized_scene_ = scene_.Serialize();
        // Reload so scripts are upt to date
        scene_.Deserialize(serialized_scene_);
        scene_.Play();
      }
      state_ = State::RUNNING;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Play");
    }
  }
  ImGui::SameLine();
  if (ImGui::TextureButton(icons_.stop.get())) {
    if (state_ != State::STOPPED) {
      scene_.Stop();
      scene_.Deserialize(serialized_scene_);
    }
    state_ = State::STOPPED;
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Stop");
  }

  ImGui::SameLine();
  ImGui::Dummy(ImVec2(20, 0));

  ImGui::SameLine();
  if (ImGui::TextureButton(icons_.move.get())) {
    editing_mode_ = EditingMode::MOVE;
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Move");
  }

  ImGui::SameLine();
  if (ImGui::TextureButton(icons_.rotate.get())) {
    editing_mode_ = EditingMode::ROTATE;
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Rotate");
  }

  ImGui::SameLine();
  if (ImGui::TextureButton(icons_.scale.get())) {
    editing_mode_ = EditingMode::SCALE;
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Scale");
  }

  ImGui::PopStyleColor();

  ImVec4 border_color(0, 0, 0, 1);
  if (state_ == State::RUNNING) {
    border_color = ImGui::GetStyle().Colors[ImGuiCol_NavHighlight];
  }
  ImVec2 available_space = ImGui::GetContentRegionAvail();
  available_space.x -= 2;
  available_space.y -= 2;
  camera_.SetAspectRatio(available_space.x / available_space.y);

  if (!scene_viewport_ ||
      (glm::ivec2(vector2(available_space)) != scene_viewport_->GetSize() &&
       !ImGui::IsMouseDown(ImGuiMouseButton_Left))) {  // TODO: this check should be optimizied. Dragging can mean a
                                                       // lot of things not necessarily resizing the window
    LogI("Creating viewport");
    CreateSceneViewport(available_space);
  }
  scene_viewport_->SetCamera(camera_, camera_transform_.CalculateMatrix());
  scene_viewport_->Render(false);

  const vector2 top_left = static_cast<vector2>(ImGui::GetWindowPos()) + static_cast<vector2>(ImGui::GetCursorPos());
  ImGui::Image(scene_viewport_->color_texture()->texture(), available_space, ImVec2(0, 1), ImVec2(1, 0),
               ImVec4(1, 1, 1, 1), border_color);
  scene_window_focused_ = ImGui::IsWindowFocused();
  if (ImGui::IsItemHovered()) {
    camera_.SetVerticalFieldOfView(camera_.vertical_field_of_view() * std::powf(2.0, -ImGui::GetIO().MouseWheel));

    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
      const vector3 world0 = scene_viewport_->DeviceCoordinatesToWorldSpace({0.0f, 0.0f});
      const vector3 world1 = scene_viewport_->DeviceCoordinatesToWorldSpace(ImGui::GetIO().MouseDelta);
      camera_transform_.Translate(world0 - world1);
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      const vector2 mouse_position = ImGui::GetMousePos();
      SceneObject* object =
          GetObjectAtPosition(scene_viewport_->DeviceCoordinatesToWorldSpace(mouse_position - top_left));
      if (object == nullptr) {
        selection_ = SelectedScene{};
        LogI("Selecting scene");
      } else {
        selection_ = SelectedObject{object->name()};
        LogI("Selecting object: {}", object->name());
      }
    }

    SceneObject* selected_object = GetSelectedObject();
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && selected_object != nullptr) {
      const vector2 mouse_position = ImGui::GetMousePos();
      move_state_.drag_start_mouse_position = scene_viewport_->DeviceCoordinatesToWorldSpace(mouse_position - top_left);

      if (selected_object->HasComponent("Transform")) {
        move_state_.original_position = selected_object->GetComponent<TransformComponent>("Transform")->translation();

        LogI("Original position: {}", move_state_.original_position);
      }
    }
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && selected_object != nullptr) {
      const vector2 mouse_position = ImGui::GetMousePos();
      const vector3 current_mouse_pos = scene_viewport_->DeviceCoordinatesToWorldSpace(mouse_position - top_left);
      const vector3 position_delta = current_mouse_pos - move_state_.drag_start_mouse_position;
      const vector3 object_position = move_state_.original_position + position_delta;

      if (selected_object->HasComponent("Transform")) {
        selected_object->GetComponent<TransformComponent>("Transform")->SetTranslation(object_position);
      }
    }
  }

  if (ImGui::BeginDragDropTarget()) {
    std::string dropped_asset_id;
    if (ImGui::AcceptDragDropAsset("texture2d", &dropped_asset_id)) {
      SceneObject* object = CreateObject(dropped_asset_id, false);
      auto texture_description = LoadTexture2DDescription(GetApplicationAssetLibrary(), dropped_asset_id);

      auto* transform = object->AddComponent<TransformComponent>("Transform");
      const vector2 mouse_position = ImGui::GetMousePos();
      transform->SetTranslation(scene_viewport_->DeviceCoordinatesToWorldSpace(mouse_position - top_left));

      auto* sprite = object->AddComponent<SpriteComponent>("Sprite");
      sprite->SetTexture(dropped_asset_id);
      sprite->SetSize({texture_description->width, texture_description->height});

      serialized_scene_ = scene_.Serialize();
      SubmitJsonFile(serialized_scene_);
    }
    ImGui::EndDragDropTarget();
  }
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

void SceneEditor::CreateNew(const std::string& asset_id) {
  GetApplicationAssetLibrary()->CreateAsset(asset_id, "scene", {std::make_pair("json", Scene().Serialize().dump())});
}

bool SceneEditor::DrawObjectList() {
  bool scene_changed = false;

  ImVec2 window_size = ImGui::GetWindowSize();
  ImGui::BeginChild("ObjectView", ImVec2(0, window_size.y / 2), true);
  if (ImGui::BeginPopupContextWindow()) {
    if (ImGui::Selectable("Create Object")) {
      CreateObject("New Object", true);
    }
    ImGui::EndPopup();
  }

  ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                       ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

  ImGuiTreeNodeFlags scene_node_flags = tree_node_flags | ImGuiTreeNodeFlags_AllowItemOverlap;
  if (std::holds_alternative<SelectedScene>(selection_)) {
    scene_node_flags |= ImGuiTreeNodeFlags_Selected;
  }
  if (ImGui::TreeNodeEx(asset_id().c_str(), scene_node_flags)) {
    if (ImGui::BeginPopupContextItem()) {
      if (ImGui::Selectable("Create Object")) {
        CreateObject("New Object", true);
      }
      ImGui::EndPopup();
    }
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      selection_ = SelectedScene{};
    }

    const std::string selected_object_name = get_with_default<SelectedObject>(selection_, SelectedObject::NONE).name;
    scene_.GetObjects(&cached_scene_objects_, true);
    for (SceneObject* object : cached_scene_objects_) {
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
        if (ImGui::InputText("Renaming", &new_object_name,
                             ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
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
    const std::string selected_object_name = get_with_default<SelectedObject>(selection_, SelectedObject::NONE).name;

    if (selected_object_name.size() != 0) {
      SceneObject* selected_object = scene_.GetObject(selected_object_name);

      ImGui::Text("%s", selected_object->name().c_str());
      ImGui::SameLine();
      if (ImGui::BeginCombo("##AddComponent", "Add Component", ImGuiComboFlags_NoArrowButton)) {
        for (const auto& component_id : SceneObjectComponent::GetRegisteredComponents()) {
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
        Serializable* component = selected_object->GetComponent(component_id);
        if (ImGui::InputSerializable(component_id.c_str(), component)) {
          object_changed = true;
        }
      }
      ImGui::EndChild();

      ImGui::BeginChild("Scene Object Buttons");

      ImGui::EndChild();
    } else {
      // if (ImGui::InputJson("Scene", &serialized_scene_, *scene_.GetSchema(),
      //                      ImGuiInputJsonFlags_IgnoreEnclosingObject)) {
      //   // TODO: move to appropriate function and use serialized scene and do not re-serialze each frame
      //   scene_.Deserialize(serialized_scene_);
      //   object_changed = true;
      // }
      for (const std::string& controller : SceneController::GetRegisteredControllers()) {
        bool controller_enabled = scene_.GetController(controller) != nullptr;
        if (ImGui::Checkbox(controller.c_str(), &controller_enabled)) {
          if (controller_enabled) {
            scene_.AddController(controller);
          } else {
            scene_.RemoveController(controller);
          }
        }
      }
      for (const std::string& controller : GetApplicationAssetLibrary()->GetAssetsWithType("scene_controller")) {
        bool controller_enabled = scene_.GetController(controller) != nullptr;
        if (ImGui::Checkbox(controller.c_str(), &controller_enabled)) {
          if (controller_enabled) {
            scene_.AddController(controller);
          } else {
            scene_.RemoveController(controller);
          }
        }
      }
    }
  }
  ImGui::EndChild();

  return object_changed;
}

void SceneEditor::CreateSceneViewport(ImVec2 size) {
  if (!scene_viewport_) {
    RenderTargetViewportDescription scene_viewport_description;
    scene_viewport_description.color_description.texture_description.width = static_cast<size_t>(size.x);
    scene_viewport_description.color_description.texture_description.height = static_cast<size_t>(size.y);
    scene_viewport_description.color_description.texture_description.format = TextureFormat::RGB_UINT8;
    scene_viewport_description.color_description.texture_description.filter = TextureFilter::POINT;
    scene_viewport_description.color_description.texture_description.mip_map_count = 0;
    scene_viewport_ = std::make_unique<RenderTargetViewport>(
        EditorWindow::instance()->context(), EditorWindow::instance()->resource_manager(), scene_viewport_description);

    scene_viewport_->AddRenderPass("Clear");
    scene_viewport_->AddRenderPass("SpriteRenderer");
    scene_viewport_->AddRenderPass("SceneEditorRenderPass");
    scene_viewport_->SetScene(&scene_);
  } else {
    scene_viewport_->Resize(size.x, size.y);
  }
}

void SceneEditor::JsonFileChanged(const json& data, const std::string& file_type) {
  if (file_type == "json") {
    serialized_scene_ = data;
    scene_.Deserialize(serialized_scene_);
  }
}

SceneObject* SceneEditor::CreateObject(const std::string& base_name, bool initiate_rename) {
  std::string object_name = base_name;
  int counter = 1;
  while (scene_.ContainsObject(object_name)) {
    counter++;
    object_name = base_name + std::to_string(counter);
  }
  selection_ = SelectedObject{object_name};
  if (initiate_rename) {
    renaming_state_ = RenamingState::STARTED_RENAMING;
  }
  return scene_.CreateObject(object_name);
}

SceneObject* SceneEditor::GetSelectedObject() {
  if (std::holds_alternative<SelectedObject>(selection_)) {
    return scene_.GetObject(std::get<SelectedObject>(selection_).name);
  } else {
    return nullptr;
  }
}

SceneObject* SceneEditor::GetObjectAtPosition(vector2 world_position) {
  SceneObject* object_at_position = nullptr;
  float z = std::numeric_limits<float>::infinity();

  for (SceneObject* object : scene_.GetObjects()) {
    vector2 size;
    if (object->HasComponent("Sprite")) {
      size = object->GetComponent<SpriteComponent>("Sprite")->size();
    }

    vector3 position(0.0f, 0.0f, 0.0f);
    if (object->HasComponent("Transform")) {
      Transform* transform = object->GetComponent<TransformComponent>("Transform");
      position = transform->translation();
      size *= vector2(transform->scale());
    }

    const vector2 half_size = size * 0.5f;

    LogI("Candidate: {}", object->name());
    LogI("World position: {}", world_position);
    LogI("Object position: {}", position);
    LogI("Object Size: {}", size);

    if (position.x - half_size.x <= world_position.x && world_position.x <= position.x + half_size.x &&
        position.y - half_size.y <= world_position.y && world_position.y <= position.y + half_size.y &&
        position.z < z) {
      object_at_position = object;
      z = position.z;
    }
  }

  return object_at_position;
}

}  // namespace editor
}  // namespace ovis