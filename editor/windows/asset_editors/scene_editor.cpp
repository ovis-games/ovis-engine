#include "scene_editor.hpp"

#include "../../editor_window.hpp"
#include "../../imgui_extensions/input_asset.hpp"
#include "../../imgui_extensions/input_serializable.hpp"
#include "../../imgui_extensions/texture_button.hpp"

#include <imgui_stdlib.h>
#include <ovis/base/transform2d_component.hpp>

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
  scene_.camera().SetAspectRatio(available_space.x / available_space.y);

  if (!scene_viewport_ || false) {  // Recreate if viewport size changed
    CreateSceneViewport(available_space);
  }
  scene_viewport_->Render(false);

  const glm::vec2 top_left =
      static_cast<glm::vec2>(ImGui::GetWindowPos()) + static_cast<glm::vec2>(ImGui::GetCursorPos());
  ImGui::Image(scene_viewport_->color_texture()->texture(), available_space, ImVec2(0, 0), ImVec2(1, 1),
               ImVec4(1, 1, 1, 1), border_color);
  scene_window_focused_ = ImGui::IsWindowFocused();
  if (ImGui::IsItemHovered()) {
    scene_.camera().SetVerticalFieldOfView(scene_.camera().vertical_field_of_view() *
                                           std::powf(2.0, -ImGui::GetIO().MouseWheel));

    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
      float movement_scaling = scene_.camera().vertical_field_of_view() / available_space.y;
      scene_.camera().transform().Translate(
          {-movement_scaling * ImGui::GetIO().MouseDelta.x, -movement_scaling * ImGui::GetIO().MouseDelta.y, 0});
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      const glm::vec2 mouse_position = ImGui::GetMousePos();
      SceneObject* object = GetObjectAtPosition(ScreenToWorld(mouse_position - top_left));
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
      const glm::vec2 mouse_position = ImGui::GetMousePos();
      move_state_.drag_start_mouse_position = ScreenToWorld(mouse_position - top_left);

      if (selected_object->HasComponent("Transform2D")) {
        move_state_.original_position =
            selected_object->GetComponent<Transform2DComponent>("Transform2D")->transform()->translation();

        LogI("Original position: ({},{})", move_state_.original_position.x, move_state_.original_position.y);
      }
    }
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && selected_object != nullptr) {
      const glm::vec2 mouse_position = ImGui::GetMousePos();
      const glm::vec2 current_mouse_pos = ScreenToWorld(mouse_position - top_left);
      const glm::vec2 position_delta = current_mouse_pos - move_state_.drag_start_mouse_position;
      const glm::vec2 object_position = move_state_.original_position + position_delta;

      if (selected_object->HasComponent("Transform2D")) {
        selected_object->GetComponent<Transform2DComponent>("Transform2D")
            ->transform()
            ->SetTranslation(glm::vec3(object_position, 0.0f));
      }

      // LogI("Mouse delta: ({},{})", object_position.x, object_position.y);
    }
  }

  if (ImGui::BeginDragDropTarget()) {
    std::string dropped_asset_id;
    if (ImGui::AcceptDragDropAsset("texture2d", &dropped_asset_id)) {
      SceneObject* object = CreateObject(dropped_asset_id);
      auto texture_description = LoadTexture2DDescription(GetApplicationAssetLibrary(), dropped_asset_id);

      auto* transform2d = object->AddComponent<Transform2DComponent>("Transform2D");
      const glm::vec2 mouse_position = ImGui::GetMousePos();
      transform2d->transform()->SetTranslation(glm::vec3(ScreenToWorld(mouse_position - top_left), 0.0f));

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
      if (ImGui::InputJson("Scene", &serialized_scene_, *scene_.GetSchema(),
                           ImGuiInputJsonFlags_IgnoreEnclosingObject)) {
        // TODO: move to appropriate function and use serialized scene and do not re-serialze each frame
        scene_.Deserialize(serialized_scene_);
        object_changed = true;
      }
    }
  }
  ImGui::EndChild();

  return object_changed;
}

void SceneEditor::CreateSceneViewport(ImVec2 size) {
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
  scene_viewport_->SetScene(&scene_);
}

void SceneEditor::JsonFileChanged(const json& data, const std::string& file_type) {
  if (file_type == "json") {
    serialized_scene_ = data;
    scene_.Deserialize(serialized_scene_);
  }
}

glm::vec2 SceneEditor::ScreenToWorld(glm::vec2 screen_position) {
  const Camera& camera = scene_.camera();
  const float vertical_field_of_view = camera.vertical_field_of_view();
  const float horizontal_field_of_view = camera.aspect_ratio() * vertical_field_of_view;
  const glm::vec2 field_of_view = {horizontal_field_of_view, vertical_field_of_view};

  const glm::vec2 view_space_position = screen_position / (glm::vec2(scene_viewport_->GetSize() - 1));
  const glm::vec2 camera_translation = glm::vec2(camera.transform().translation());
  const glm::vec2 world_position = (view_space_position - glm::vec2(0.5f, 0.5f)) * field_of_view + camera_translation;

  return world_position;
}

SceneObject* SceneEditor::CreateObject(const std::string& base_name) {
  std::string object_name = base_name;
  int counter = 1;
  while (scene_.ContainsObject(object_name)) {
    counter++;
    object_name = base_name + std::to_string(counter);
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

SceneObject* SceneEditor::GetObjectAtPosition(glm::vec2 world_position) {
  SceneObject* object_at_position = nullptr;

  for (SceneObject* object : scene_.GetObjects()) {
    glm::vec2 size;
    if (object->HasComponent("Sprite")) {
      size = object->GetComponent<SpriteComponent>("Sprite")->size();
    }

    glm::vec2 position(0.0f, 0.0f);
    if (object->HasComponent("Transform2D")) {
      Transform* transform = object->GetComponent<Transform2DComponent>("Transform2D")->transform();
      position = transform->translation();
      size *= glm::vec2(transform->scale());
    }

    const glm::vec2 half_size = size * 0.5f;

    LogI("World position: ({},{})", world_position.x, world_position.y);
    LogI("Object position: ({},{})", position.x, position.y);
    LogI("Object Size: ({},{})", size.x, size.y);

    if (position.x - half_size.x <= world_position.x && world_position.x <= position.x + half_size.x &&
        position.y - half_size.y <= world_position.y && world_position.y <= position.y + half_size.y) {
      object_at_position = object;
    }
  }

  return object_at_position;
}

}  // namespace editor
}  // namespace ovis