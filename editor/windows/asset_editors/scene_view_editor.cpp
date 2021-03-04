#include "scene_view_editor.hpp"

#include "../../editor_window.hpp"
#include "../../imgui_extensions/input_asset.hpp"
#include "../../imgui_extensions/input_serializable.hpp"
#include "../../imgui_extensions/texture_button.hpp"
#include "editing_controllers/editor_camera_controller.hpp"

#include <imgui_stdlib.h>
#include <ovis/base/transform_component.hpp>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/utils.hpp>
#include <ovis/engine/input.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/scene_controller.hpp>
#include <ovis/engine/scene_object.hpp>
#include <ovis/rendering2d/sprite_component.hpp>
#include <ovis/physics2d/physics2d_debug_layer.hpp>

namespace ovis {
namespace editor {

namespace {

MouseButton GetMouseButtonFromImGuiIndex(int button) {
  // clang-format off
  switch (button) {
    case ImGuiMouseButton_Left: return MouseButton::LEFT;
    case ImGuiMouseButton_Middle: return MouseButton::MIDDLE;
    case ImGuiMouseButton_Right: return MouseButton::RIGHT;
    case 3: return MouseButton::EXTRA1;
    case 4: return MouseButton::EXTRA2;
    default: SDL_assert(false); return MouseButton::LEFT; // Keep clang happy
  }
  // clang-format off
}

}  // namespace

SceneViewEditor::SceneViewEditor(const std::string& scene_asset) : AssetEditor(scene_asset) {
  Lua::on_error.Subscribe([this](const std::string&) {
    if (run_state() == RunState::RUNNING) {
      ChangeRunState(RunState::PAUSED);
    }
  });

  icons_.play = LoadTexture2D("icon-play", EditorWindow::instance()->context());
  icons_.pause = LoadTexture2D("icon-pause", EditorWindow::instance()->context());
  icons_.stop = LoadTexture2D("icon-stop", EditorWindow::instance()->context());
  icons_.move = LoadTexture2D("icon-move", EditorWindow::instance()->context());
  icons_.rotate = LoadTexture2D("icon-rotate", EditorWindow::instance()->context());
  icons_.scale = LoadTexture2D("icon-scale", EditorWindow::instance()->context());
  icons_.eye = LoadTexture2D("icon-eye", EditorWindow::instance()->context());

  editing_scene()->Play();
}

void SceneViewEditor::Update(std::chrono::microseconds delta_time) {
  if (run_state() == RunState::RUNNING) {
    game_scene_.Update(delta_time);
    serialized_scene_editing_copy_ = game_scene_.Serialize();
  } else {
    editing_scene_.Update(delta_time);
  }
}

void SceneViewEditor::ProcessViewportInputEvent(Event* event) {
  if (run_state() == RunState::RUNNING) {
    game_scene_.ProcessEvent(event);
    serialized_scene_editing_copy_ = game_scene_.Serialize();
  } else {
    editing_scene_.ProcessEvent(event);
  }
}

void SceneViewEditor::ChangeRunState(RunState new_state) {
  switch (new_state)
  {
  case RunState::RUNNING:
    if (run_state() == RunState::PAUSED) {
      // Nothing to do here
      run_state_ = RunState::RUNNING;
    } else if (run_state() == RunState::STOPPED) {
      SubmitChangesToScene();
      game_scene()->Deserialize(GetCurrentJsonFileState());
      game_scene()->Play();
      run_state_ = RunState::RUNNING;
    }
    break;

  case RunState::PAUSED:
    if (run_state() == RunState::STOPPED) {
      // This does not make any sense!
      SDL_assert(false);
      run_state_ = RunState::PAUSED;
    } else if (run_state() == RunState::RUNNING) {
      // Nothing to do here
      run_state_ = RunState::PAUSED;
    }
    break;

  case RunState::STOPPED:
    if (run_state() == RunState::RUNNING || run_state() == RunState::PAUSED) {
      game_scene()->Stop();
      serialized_scene_editing_copy_ = GetCurrentJsonFileState();
      game_scene()->Deserialize(serialized_scene_editing_copy_);
      run_state_= RunState::STOPPED;
    }
    break;
  }
}

void SceneViewEditor::DrawContent() {
  DrawToolbar();
  DrawViewport();
}

void SceneViewEditor::DrawToolbar() {
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(48, 48, 48)));
  if (run_state() == RunState::RUNNING) {
    if (ImGui::TextureButton(icons_.pause.get())) {
      ChangeRunState(RunState::PAUSED);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Pause");
    }
  } else {
    if (ImGui::TextureButton(icons_.play.get())) {
      ChangeRunState(RunState::RUNNING);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Run");
    }
  }
  ImGui::SameLine();
  if (ImGui::TextureButton(icons_.stop.get())) {
    ChangeRunState(RunState::STOPPED);
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Stop");
  }

  ImGui::SameLine();
  ImGui::Dummy(ImVec2(20, 0));

  ImGui::SameLine();
  if (ImGui::TextureButton(icons_.move.get())) {
    SDL_assert(false);
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Move");
  }

  ImGui::SameLine();
  if (ImGui::TextureButton(icons_.rotate.get())) {
    SDL_assert(false);
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Rotate");
  }

  ImGui::SameLine();
  if (ImGui::TextureButton(icons_.scale.get())) {
    SDL_assert(false);
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Scale");
  }

  ImGui::SameLine();
  ImGui::Dummy(ImVec2(20, 0));

  ImGui::SameLine();
  ImVec2 eyebutton_pos = ImGui::GetCursorScreenPos();
  if (ImGui::TextureButton(icons_.eye.get())) {
    ImGui::OpenPopup("Overlay Settings");
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Overlay settings");
  }
  ImVec2 next_line_pos = ImGui::GetCursorScreenPos();
  ImGui::SetNextWindowPos({eyebutton_pos.x, next_line_pos.y});
  if (ImGui::BeginPopup("Overlay Settings")) {
    Physics2DDebugLayer* overlay = scene_viewport_->GetRenderPass<Physics2DDebugLayer>("Physics2DDebugLayer");
    if (ImGui::BeginMenu("Physics World 2D Debug Layer")) {
      auto draw_setting = [overlay](const char* label, uint32 flag) {
        bool display = (overlay->GetFlags() & flag) != 0;
        if (ImGui::Checkbox(label, &display)) {
          if (display) {
            overlay->AppendFlags(flag);
          } else {
            overlay->ClearFlags(flag);
          }
        }
      };
      draw_setting("Display Shapes", b2Draw::e_shapeBit);
      draw_setting("Display Joints", b2Draw::e_jointBit);
      draw_setting("Display Bounding Boxes", b2Draw::e_aabbBit);
      draw_setting("Display Broad Phase Pairs", b2Draw::e_pairBit);
      draw_setting("Display Center of Mass", b2Draw::e_centerOfMassBit);
      float transform_size = overlay->GetTransformLineLength();
      if (ImGui::DragFloat("Transform Size", &transform_size, 1.0f, 0.0f, std::numeric_limits<float>::infinity(),
                           "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
        overlay->SetTransformLineLength(transform_size);
      }

      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }

  ImGui::PopStyleColor();
}

void SceneViewEditor::DrawViewport() {
  ImVec4 border_color(0, 0, 0, 1);
  if (run_state() != RunState::STOPPED) {
    border_color.x = 1;
    border_color.y = 1;
  }
  ImVec2 available_space = ImGui::GetContentRegionAvail();
  available_space.x -= 2;
  available_space.y -= 2;

  if (!scene_viewport_ ||
      (glm::ivec2(vector2(available_space)) != scene_viewport_->GetSize() &&
       !ImGui::IsMouseDown(ImGuiMouseButton_Left))) {  // TODO: this check should be optimizied. Dragging can mean a
                                                       // lot of things not necessarily resizing the window
    LogI("Creating viewport");
    CreateSceneViewport(available_space);
  }
  scene_viewport_->Render(false);

  const vector2 top_left = static_cast<vector2>(ImGui::GetWindowPos()) + static_cast<vector2>(ImGui::GetCursorPos());
  ImGui::Image(scene_viewport_->color_texture()->texture(), available_space, ImVec2(0, 1), ImVec2(1, 0),
               ImVec4(1, 1, 1, 1), border_color);
  scene_window_focused_ = ImGui::IsWindowFocused();

  const auto& io = ImGui::GetIO();
  if (ImGui::IsItemHovered()) {
    if (io.MouseWheel != 0 || io.MouseWheelH != 0) {
      MouseWheelEvent mouse_wheel_event(io.MouseWheelH, io.MouseWheel);
      ProcessViewportInputEvent(&mouse_wheel_event);
    }
    const vector2 mouse_position = vector2(ImGui::GetMousePos()) - top_left;
    if (mouse_position != latest_mouse_position_) {
      const vector2 relative_position =
          glm::any(glm::isnan(latest_mouse_position_)) ? vector2(0.0f, 0.0f) : mouse_position - latest_mouse_position_;
      MouseMoveEvent mouse_move_event(scene_viewport_.get(), mouse_position, relative_position);
      ProcessViewportInputEvent(&mouse_move_event);
      latest_mouse_position_ = mouse_position;
    }
    for (int i = 0; i < 5; ++i) {
      if (ImGui::IsMouseClicked(i)) {
        MouseButtonPressEvent mouse_button_event(scene_viewport_.get(), mouse_position, GetMouseButtonFromImGuiIndex(i));
        ProcessViewportInputEvent(&mouse_button_event);
      }
      if (ImGui::IsMouseReleased(i)) {
        MouseButtonReleaseEvent mouse_button_event(scene_viewport_.get(), mouse_position, GetMouseButtonFromImGuiIndex(i));
        ProcessViewportInputEvent(&mouse_button_event);
      }
    }
  } else {
    latest_mouse_position_ = vector2(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN());
  }

  if (ImGui::BeginDragDropTarget()) {
    std::string dropped_asset_id;

    if (run_state() != RunState::RUNNING && ImGui::AcceptDragDropAsset("texture2d", &dropped_asset_id)) {
        SceneObject* object = CreateObject(dropped_asset_id, false);
        auto texture_description = LoadTexture2DDescription(GetApplicationAssetLibrary(), dropped_asset_id);

        auto* transform = object->AddComponent<TransformComponent>("Transform");
        const vector2 mouse_position = ImGui::GetMousePos();
        transform->SetTranslation(scene_viewport_->DeviceCoordinatesToWorldSpace(mouse_position - top_left));

        auto* sprite = object->AddComponent<SpriteComponent>("Sprite");
        sprite->SetTexture(dropped_asset_id);
        sprite->SetSize({texture_description->width, texture_description->height});

        SubmitChangesToScene();
    }
    ImGui::EndDragDropTarget();
  }
}

void SceneViewEditor::DrawInspectorContent() {
  DrawObjectTree();
  DrawSelectionProperties();
}

void SceneViewEditor::DrawObjectTree() {
  ImVec2 available_content_region = ImGui::GetContentRegionAvail();
  if (ImGui::BeginChild("ObjectView", ImVec2(0, available_content_region.y / 2), true)) {
    if (ImGui::BeginPopupContextWindow()) {
      if (ImGui::Selectable("Create Object")) {
        CreateObject("New Object", true);
      }
      ImGui::EndPopup();
    }

    // If the selected object does not exist anymore, remove the selection
    if (selected_object_.has_value() && !game_scene()->ContainsObject(*selected_object_)) {
      selected_object_.reset();
    }

    ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

    ImGuiTreeNodeFlags scene_node_flags = tree_node_flags | ImGuiTreeNodeFlags_AllowItemOverlap;
    if (!selected_object_.has_value()) {
      // If there is no selected object, the scene is selected
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
        selected_object_.reset();
      }

      game_scene()->GetObjects(&cached_scene_objects_, true);
      for (SceneObject* object : cached_scene_objects_) {
        SDL_assert(object != nullptr);

        const bool is_object_selected = selected_object_.has_value() && object->name() == *selected_object_;

        ImGuiTreeNodeFlags scene_object_flags = tree_node_flags | ImGuiTreeNodeFlags_Leaf;
        if (is_object_selected) {
          scene_object_flags |= ImGuiTreeNodeFlags_Selected;
        }

        if (renaming_state_ == RenamingState::IS_NOT_RENAMING || !is_object_selected) {
          // This object is currently not beeing renamed
          if (ImGui::TreeNodeEx(object->name().c_str(), scene_object_flags)) {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
              selected_object_ = object->name();
            }
            if (ImGui::BeginPopupContextItem()) {
              if (ImGui::Selectable("Rename")) {
                renaming_state_ = RenamingState::STARTED_RENAMING;
              }
              if (ImGui::Selectable("Remove")) {
                game_scene()->DeleteObject(object->name());
                SubmitChangesToScene();
              }
              ImGui::EndPopup();
            }
            ImGui::TreePop();
          }
        } else {
          // This object is currently beeing renamed
          std::string new_object_name = object->name();
          ImGui::TreePush(object->name().c_str());
          ImGui::PushItemWidth(-1);
          if (ImGui::InputText("Renaming", &new_object_name,
                               ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (new_object_name != object->name()) {
              // TODO: check for validity: alphanumeric+underscore+space
              if (game_scene()->ContainsObject(new_object_name)) {
                // TODO: Show message box
              } else {
                game_scene()->CreateObject(new_object_name, object->Serialize());
                game_scene()->DeleteObject(object->name());
                SubmitChangesToScene();
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
  }
  ImGui::EndChild();
}

void SceneViewEditor::DrawSelectionProperties() {
  ImVec2 available_content_region = ImGui::GetContentRegionAvail();
  if (ImGui::BeginChild("SelectionProperties", ImVec2(0, available_content_region.y), true)) {
    if (selected_object_.has_value()) {
      DrawSceneObjectProperties();
    } else {
      DrawSceneProperties();
    }
  }
  ImGui::EndChild();
}

void SceneViewEditor::DrawSceneObjectProperties() {
  SceneObject* selected_object = GetSelectedObject();

  if (!selected_object) {
    return;
  }

  for (const auto& component_id : selected_object->GetComponentIds()) {
    Serializable* component = selected_object->GetComponent(component_id);

    // Get component path
    const json::json_pointer component_path = GetComponentPath(*selected_object_, component_id);
    if (serialized_scene_editing_copy_.contains(component_path)) {
      json& serialized_component = serialized_scene_editing_copy_[component_path];
      const ovis::json* component_schema = component->GetSchema();

      if (ImGui::InputJson(component_id.c_str(), &serialized_component,
                           component_schema ? *component_schema : ovis::json{})) {
        component->Deserialize(serialized_component);
      }
      if (ImGui::IsItemDeactivated()) {
        // After editing is finished reserialize the component so the input gets "validated"
        SubmitChangesToScene();
      }
    }
  }
}

void SceneViewEditor::DrawSceneProperties() {
  for (const std::string& controller : SceneController::GetRegisteredControllers()) {
    bool controller_enabled = game_scene()->GetController(controller) != nullptr;
    if (ImGui::Checkbox(controller.c_str(), &controller_enabled)) {
      if (controller_enabled) {
        game_scene()->AddController(controller);
      } else {
        game_scene()->RemoveController(controller);
      }
      SubmitChangesToScene();
    }
  }
  for (const std::string& controller : GetApplicationAssetLibrary()->GetAssetsWithType("scene_controller")) {
    bool controller_enabled = game_scene()->GetController(controller) != nullptr;
    if (ImGui::Checkbox(controller.c_str(), &controller_enabled)) {
      if (controller_enabled) {
        game_scene()->AddController(controller);
      } else {
        game_scene()->RemoveController(controller);
      }
      SubmitChangesToScene();
    }
  }
}

void SceneViewEditor::SetSerializedScene(const json& data) {
  ChangeRunState(RunState::STOPPED);
  game_scene()->Deserialize(data);
  SubmitJsonFile(data);
}

void SceneViewEditor::SubmitChangesToScene() {
  if (run_state() == RunState::STOPPED) {
    serialized_scene_editing_copy_ = game_scene()->Serialize();
    SubmitJsonFile(serialized_scene_editing_copy_);
  }
}

void SceneViewEditor::CreateSceneViewport(ImVec2 size) {
  if (!scene_viewport_) {
    RenderTargetViewportDescription scene_viewport_description;
    scene_viewport_description.color_description.texture_description.width = static_cast<size_t>(size.x);
    scene_viewport_description.color_description.texture_description.height = static_cast<size_t>(size.y);
    scene_viewport_description.color_description.texture_description.format = TextureFormat::RGB_UINT8;
    scene_viewport_description.color_description.texture_description.filter = TextureFilter::POINT;
    scene_viewport_description.color_description.texture_description.mip_map_count = 0;
    scene_viewport_ = std::make_unique<RenderTargetViewport>(
        EditorWindow::instance()->context(), EditorWindow::instance()->resource_manager(), scene_viewport_description);

    
    editing_scene()->AddController<EditorCameraController>(scene_viewport_.get());

    scene_viewport_->AddRenderPass("Clear");
    scene_viewport_->AddRenderPass("SpriteRenderer");
    scene_viewport_->AddRenderPass("SceneViewEditorRenderPass");
    scene_viewport_->AddRenderPass("Physics2DDebugLayer");
    scene_viewport_->SetScene(game_scene());
  } else {
    scene_viewport_->Resize(size.x, size.y);
  }
}

SceneObject* SceneViewEditor::CreateObject(const std::string& base_name, bool initiate_rename) {
  std::string object_name = base_name;
  int counter = 1;
  while (game_scene()->ContainsObject(object_name)) {
    counter++;
    object_name = base_name + std::to_string(counter);
  }
  selected_object_ = object_name;
  if (initiate_rename) {
    renaming_state_ = RenamingState::STARTED_RENAMING;
  }
  return game_scene()->CreateObject(object_name);
}

SceneObject* SceneViewEditor::GetSelectedObject() {
  if (selected_object_.has_value()) {
    return game_scene()->GetObject(*selected_object_);
  } else {
    return nullptr;
  }
}

SceneObject* SceneViewEditor::GetObjectAtPosition(vector2 world_position) {
  SceneObject* object_at_position = nullptr;
  float z = std::numeric_limits<float>::infinity();

  for (SceneObject* object : game_scene()->GetObjects()) {
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