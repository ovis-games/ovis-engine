#include "scene_view_editor.hpp"

#include "../../clipboard.hpp"
#include "../../editor_window.hpp"
#include "../../imgui_extensions/input_asset.hpp"
#include "../../imgui_extensions/input_serializable.hpp"
#include "../../imgui_extensions/texture_button.hpp"
#include "editing_controllers/editor_camera_controller.hpp"
#include "editing_controllers/object_selection_controller.hpp"
#include "editing_controllers/physics2d_shape_controller.hpp"
#include "editing_controllers/transformation_tools_controller.hpp"
#include "editor_render_passes/physics2d_shape_renderer.hpp"
#include "editor_render_passes/selected_object_bounding_box.hpp"
#include "editor_render_passes/transformation_tools_renderer.hpp"

#include <imgui_stdlib.h>

#include <ovis/utils/utils.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/scene_controller.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/transform.hpp>
#include <ovis/input/mouse_button.hpp>
#include <ovis/input/mouse_events.hpp>
#include <ovis/imgui/imgui_start_frame_controller.hpp>

namespace ovis {
namespace editor {

namespace {

MouseButton GetMouseButtonFromImGuiIndex(int button) {
  // clang-format off
  switch (button) {
    case ImGuiMouseButton_Left: return MouseButton::Left();
    case ImGuiMouseButton_Middle: return MouseButton::Middle();
    case ImGuiMouseButton_Right: return MouseButton::Right();
    case 3: return MouseButton::Four();
    case 4: return MouseButton::Five();
    default: SDL_assert(false); return MouseButton::Left(); // Keep clang happy
  }
  // clang-format off
}

}  // namespace

SceneViewEditor::SceneViewEditor(const std::string& scene_asset) : AssetEditor(scene_asset) {
  editing_scene()->Play();
  AddEditorController<ObjectSelectionController>();
  AddEditorController<TransformationToolsController>();
  AddEditorController<EditorCameraController>();
  AddEditorController<Physics2DShapeController>();

  SubscribeToEvent(LuaErrorEvent::TYPE);
}

void SceneViewEditor::Update(std::chrono::microseconds delta_time) {
  AssetEditor::Update(delta_time);
  if (run_state() == RunState::RUNNING) {
    game_scene_.Update(delta_time);
    serialized_scene_editing_copy_ = game_scene_.Serialize();
  } else {
    editing_scene_.Update(delta_time);
  }
}

void SceneViewEditor::ProcessEvent(Event* event) {
  AssetEditor::ProcessEvent(event);
  if (event->is_propagating()) {
    if (event->type() == LuaErrorEvent::TYPE) {
      if (run_state() == RunState::RUNNING) {
        ChangeRunState(RunState::PAUSED);
      }
    }
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
    switch (run_state()) {
      case RunState::STOPPED:
        SubmitChangesToScene();
        game_scene()->Deserialize(GetCurrentJsonFileState());
        game_scene()->Play();
        [[fallthrough]];
  
      case RunState::PAUSED:
        run_state_ = RunState::RUNNING;
        break;

      case RunState::RUNNING:
        // nothing to do here
        break;
    }
    break;

  case RunState::PAUSED:
    if (run_state() == RunState::STOPPED) {
      // This does not make any sense!
      SDL_assert(false);
      run_state_ = RunState::PAUSED;
    } else if (run_state() == RunState::RUNNING) {
      run_state_ = RunState::PAUSED;
    }
    break;

  case RunState::STOPPED:
    switch (run_state()) {
      case RunState::RUNNING:
        [[fallthrough]];

      case RunState::PAUSED:
        game_scene()->Stop();
        serialized_scene_editing_copy_ = GetCurrentJsonFileState();
        game_scene()->Deserialize(serialized_scene_editing_copy_);
        run_state_= RunState::STOPPED;
        break;

      case RunState::STOPPED:
        // nothing to do here
        break;
    }
    break;
  }
}


SceneObject* SceneViewEditor::GetSelectedObject() {
  auto* object_selection_controller = editing_scene()->GetController<ObjectSelectionController>();
  SDL_assert(object_selection_controller != nullptr);
  return object_selection_controller->selected_object();
}

void SceneViewEditor::CopySelectedObjectToClipboard() {
  SceneObject* object = GetSelectedObject();
  if (object != nullptr) {
    SetClipboardData(object->name(), "application/scene_object_name");
    SetClipboardData(object->Serialize().dump(), "application/scene_object_data");
  }
}

SceneObject* SceneViewEditor::PasteObjectFromClipboard() {
  const auto object_name = GetClipboardData("application/scene_object_name");
  const auto serialized_object = GetClipboardData("application/scene_object_data");
  if (object_name && serialized_object) {
    auto object = game_scene()->CreateObject(*object_name, json::parse(*serialized_object));

    auto* object_selection_controller = editing_scene()->GetController<ObjectSelectionController>();
    SDL_assert(object_selection_controller != nullptr);
    object_selection_controller->SelectObject(object);

    SubmitChangesToScene();
    return object;
  } else {
    return nullptr;
  }
}

void SceneViewEditor::DrawContent() {
  DrawToolbar();
  DrawViewport();
}

void SceneViewEditor::DrawToolbar() {
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(48, 48, 48)));

  ImFont* font_awesome = scene()->GetController<ImGuiStartFrameController>()->GetFont("FontAwesomeSolid");

  if (run_state() == RunState::RUNNING) {
    ImGui::PushFont(font_awesome);
    if (ImGui::Button("\uf04c")) {
      ChangeRunState(RunState::PAUSED);
    }
    ImGui::PopFont();
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Pause");
    }
  } else {
    ImGui::PushFont(font_awesome);
    if (ImGui::Button("\uf04b")) {
      ChangeRunState(RunState::RUNNING);
    }
    ImGui::PopFont();
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Run");
    }
  }
  ImGui::SameLine();
  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf04d")) {
    ChangeRunState(RunState::STOPPED);
  }
  ImGui::PopFont();
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Stop");
  }

  ImGui::SameLine();
  ImGui::Dummy(ImVec2(20, 0));

  auto transformation_controller = editing_scene()->GetController<TransformationToolsController>();

  ImGui::SameLine();
  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf0b2")) {
    transformation_controller->SelectTransformationType(TransformationToolsController::TransformationType::MOVE);
  }
  ImGui::PopFont();
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Move");
  }

  ImGui::SameLine();
  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf2f1")) {
    transformation_controller->SelectTransformationType(TransformationToolsController::TransformationType::ROTATE);
  }
  ImGui::PopFont();
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Rotate");
  }

  ImGui::SameLine();
  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf424")) {
    transformation_controller->SelectTransformationType(TransformationToolsController::TransformationType::SCALE);
  }
  ImGui::PopFont();
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Scale");
  }

  ImGui::SameLine();
  ImGui::PushFont(font_awesome);
  const bool world_coordinate_system = transformation_controller->coordinate_system() == TransformationToolsController::CoordinateSystem::WORLD;
  if (ImGui::Button(world_coordinate_system ? "\uf1b2" : "\uf0ac")) {
    transformation_controller->SwitchCoordinateSystem();
  }
  ImGui::PopFont();
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip(world_coordinate_system ? "Switch to local coordinate system" : "Switch to global coordinate system");
  }

  ImGui::SameLine();
  ImGui::Dummy(ImVec2(20, 0));

  ImGui::SameLine();
  ImVec2 eyebutton_pos = ImGui::GetCursorScreenPos();
  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf06e")) {
    ImGui::OpenPopup("Overlay Settings");
  }
  ImGui::PopFont();
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Overlay settings");
  }
  ImVec2 next_line_pos = ImGui::GetCursorScreenPos();
  ImGui::SetNextWindowPos({eyebutton_pos.x, next_line_pos.y});
  if (ImGui::BeginPopup("Overlay Settings")) {
  //   Physics2DDebugLayer* overlay = scene_viewport_->GetRenderPass<Physics2DDebugLayer>("Physics2DDebugLayer");
  //   if (ImGui::BeginMenu("Physics World 2D Debug Layer")) {
  //     auto draw_setting = [overlay](const char* label, uint32 flag) {
  //       bool display = (overlay->GetFlags() & flag) != 0;
  //       if (ImGui::Checkbox(label, &display)) {
  //         if (display) {
  //           overlay->AppendFlags(flag);
  //         } else {
  //           overlay->ClearFlags(flag);
  //         }
  //       }
  //     };
  //     draw_setting("Display Shapes", b2Draw::e_shapeBit);
  //     draw_setting("Display Joints", b2Draw::e_jointBit);
  //     draw_setting("Display Bounding Boxes", b2Draw::e_aabbBit);
  //     draw_setting("Display Broad Phase Pairs", b2Draw::e_pairBit);
  //     draw_setting("Display Center of Mass", b2Draw::e_centerOfMassBit);
  //     float transform_size = overlay->GetTransformLineLength();
  //     if (ImGui::DragFloat("Transform Size", &transform_size, 1.0f, 0.0f, std::numeric_limits<float>::infinity(),
  //                          "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
  //       overlay->SetTransformLineLength(transform_size);
  //     }

  //     ImGui::EndMenu();
  //   }
    ImGui::EndPopup();
  }

  ImGui::PopStyleColor();
}

void SceneViewEditor::DrawViewport() {
  ImGuiIO& io = ImGui::GetIO();
  const bool control_or_command = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;

  ImVec4 border_color(0, 0, 0, 1);
  if (run_state() != RunState::STOPPED) {
    border_color.x = 1;
    border_color.y = 1;
  }
  ImVec2 available_space = ImGui::GetContentRegionAvail();
  available_space.x -= 2;
  available_space.y -= 2;

  if (!scene_viewport_ ||
      (Vector2{available_space.x, available_space.y}) != scene_viewport_->GetDimensions() &&
       !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {  // TODO: this check should be optimizied. Dragging can mean a
                                                       // lot of things not necessarily resizing the window
    LogI("Creating viewport");
    CreateSceneViewport(available_space);
  }
  scene_viewport_->Render();

  const Vector2 top_left = static_cast<Vector2>(ImGui::GetWindowPos()) + static_cast<Vector2>(ImGui::GetCursorPos());
  ImGui::Image(scene_viewport_->color_texture()->texture()->id(), available_space, ImVec2(0, 1), ImVec2(1, 0),
               ImVec4(1, 1, 1, 1), border_color);
  ImGui::SetItemDefaultFocus();
  const Vector2 mouse_position = Vector2(ImGui::GetMousePos()) - top_left;
  
  if (ImGui::IsWindowFocused() && control_or_command && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C))) {
    CopySelectedObjectToClipboard();
  }
  if (ImGui::IsWindowFocused() && control_or_command && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V))) {
    SceneObject* object = PasteObjectFromClipboard();
    if (object && object->HasComponent("Transform")) {
      Transform* transform = object->GetComponent<Transform>("Transform");
      SDL_assert(transform != nullptr);
      const Vector3 screen_space_position = scene_viewport_->WorldSpacePositionToScreenSpace(transform->position());
      const Vector3 new_screen_space_position = Vector3::FromVector2(mouse_position, screen_space_position.z);
      const Vector3 new_world_space_position = scene_viewport_->ScreenSpacePositionToWorldSpace(new_screen_space_position);
      transform->SetPosition(new_world_space_position);
      SubmitChangesToScene();
    }
  }
  if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete))) {
    LogI("Delete object");
    game_scene()->DeleteObject(GetSelectedObject());
    SubmitChangesToScene();
  }

  if (ImGui::IsItemHovered()) {
    if (io.MouseWheel != 0 || io.MouseWheelH != 0) {
      MouseWheelEvent mouse_wheel_event({io.MouseWheelH, io.MouseWheel});
      ProcessViewportInputEvent(&mouse_wheel_event);
    }
    if (mouse_position != latest_mouse_position_) {
      const Vector2 relative_position =
          std::isnan(latest_mouse_position_.x) || std::isnan(latest_mouse_position_.y) ? Vector2::Zero() : mouse_position - latest_mouse_position_;
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
    latest_mouse_position_ = Vector2::NotANumber();
  }

  // if (ImGui::BeginDragDropTarget()) {
  //   std::string dropped_asset_id;

  //   if (run_state() != RunState::RUNNING && ImGui::AcceptDragDropAsset("texture2d", &dropped_asset_id)) {
  //       SceneObject* object = CreateObject(dropped_asset_id, false);
  //       auto texture_description = LoadTexture2DDescription(GetApplicationAssetLibrary(), dropped_asset_id);

  //       auto* transform = object->AddComponent<TransformComponent>("Transform");
  //       const Vector2 mouse_position = ImGui::GetMousePos();
  //       transform->SetPosition(scene_viewport_->DeviceCoordinatesToWorldSpace(mouse_position - top_left));

  //       auto* sprite = object->AddComponent<SpriteComponent>("Sprite");
  //       sprite->SetTexture(dropped_asset_id);
  //       sprite->SetSize({static_cast<float>(texture_description->width), static_cast<float>(texture_description->height)});

  //       SubmitChangesToScene();
  //   }
  //   ImGui::EndDragDropTarget();
  // }
}

void SceneViewEditor::DrawInspectorContent() {
  DrawObjectTree();
  ImGui::Separator();
  DrawSelectionProperties();
}

void SceneViewEditor::DrawObjectTree() {
  ImGuiIO& io = ImGui::GetIO();
  const bool control_or_command = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;

  auto* object_selection_controller = editing_scene()->GetController<ObjectSelectionController>();

  ImVec2 available_content_region = ImGui::GetContentRegionAvail();
  if (ImGui::BeginChild("ObjectView", ImVec2(0, available_content_region.y / 2), false)) {
    if (ImGui::BeginPopupContextWindow()) {
      if (ImGui::Selectable("Create Object")) {
        CreateObject("New Object", true);
      }
      ImGui::EndPopup();
    }
  
    if (ImGui::IsWindowFocused() && control_or_command && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V))) {
      PasteObjectFromClipboard();
    }

    ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

    ImGuiTreeNodeFlags scene_node_flags = tree_node_flags | ImGuiTreeNodeFlags_AllowItemOverlap;
    if (!object_selection_controller->has_selected_object()) {
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
        object_selection_controller->ClearSelection();
      }

      game_scene()->GetObjects(&cached_scene_objects_, true);
      for (SceneObject* object : cached_scene_objects_) {
        SDL_assert(object != nullptr);
        SDL_assert(game_scene()->ContainsObject(object->name()));

        const bool is_object_selected = object_selection_controller->selected_object_name() == object->name();

        ImGuiTreeNodeFlags scene_object_flags = tree_node_flags | ImGuiTreeNodeFlags_Leaf;
        if (is_object_selected) {
          scene_object_flags |= ImGuiTreeNodeFlags_Selected;
        }

        if (renaming_state_ == RenamingState::IS_NOT_RENAMING || !is_object_selected) {
          // This object is currently not beeing renamed
          if (ImGui::TreeNodeEx(object->name().c_str(), scene_object_flags)) {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
              object_selection_controller->SelectObject(object);
              SDL_assert(object_selection_controller->selected_object_name() == object->name());
            }
            if (ImGui::IsItemFocused() && control_or_command && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C))) {
              CopySelectedObjectToClipboard();
            }
            if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete))) {
              game_scene()->DeleteObject(GetSelectedObject());
              SubmitChangesToScene();
            }

            if (ImGui::BeginPopupContextItem()) {
              if (ImGui::Selectable("Rename")) {
                renaming_state_ = RenamingState::STARTED_RENAMING;
              }
              if (ImGui::Selectable("Duplicate")) {
                game_scene()->CreateObject(object->name(), object->Serialize());
                SubmitChangesToScene();
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
  auto* object_selection_controller = editing_scene()->GetController<ObjectSelectionController>();

  if (object_selection_controller->has_selected_object()) {
    DrawSceneObjectProperties();
  } else {
    DrawSceneProperties();
  }
}

void SceneViewEditor::DrawSceneObjectProperties() {
  auto* object_selection_controller = editing_scene()->GetController<ObjectSelectionController>();

  SceneObject* selected_object = object_selection_controller->selected_object();
  SDL_assert(selected_object != nullptr);

  if (!selected_object) {
    return;
  }

  ImFont* font_awesome = scene()->GetController<ImGuiStartFrameController>()->GetFont("FontAwesomeSolid");

  ImGui::Text("Components of %s:", selected_object->name().c_str());
  ImGui::SameLine();

  ImVec2 button_position = ImGui::GetCursorScreenPos();
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(48, 48, 48)));
  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf0fe")) {
    ImGui::OpenPopup("Add Component");
  }
  ImGui::PopFont();
  ImGui::PopStyleColor();
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Add Component");
  }
  
  ImVec2 next_line_pos = ImGui::GetCursorScreenPos();
  ImGui::SetNextWindowPos({button_position.x, next_line_pos.y});
  if (ImGui::BeginPopup("Add Component")) {
    for (const auto& component_id : SceneObjectComponent::registered_ids()) {
      if (!selected_object->HasComponent(component_id)) {
        if (ImGui::Selectable(component_id.c_str())) {
          selected_object->AddComponent(component_id);
          SubmitChangesToScene();
        }
      }
    }
    ImGui::EndPopup();
  }

  ImVec2 available_content_region = ImGui::GetContentRegionAvail();
  if (ImGui::BeginChild("SceneObjectComponents", ImVec2(0, available_content_region.y), false)) {
    for (const auto& component_id : selected_object->GetComponentIds()) {
      Serializable* component = selected_object->GetComponent(component_id);

      // Get component path
      const json::json_pointer component_path = GetComponentPath(object_selection_controller->selected_object_name(), component_id);
      if (serialized_scene_editing_copy_.contains(component_path)) {
        json& serialized_component = serialized_scene_editing_copy_[component_path];
        const ovis::json* component_schema = component->GetSchema();

        bool keep = true;
        if (ImGui::InputJson(component_id.c_str(), &serialized_component,
                            component_schema ? *component_schema : ovis::json{}, &keep)) {
          if (!keep) {
            selected_object->RemoveComponent(component_id);
            SubmitChangesToScene();
          } else {
            component->Deserialize(serialized_component);
          }
        }
        if (ImGui::IsItemDeactivated()) {
          // After editing is finished reserialize the component so the input gets "validated"
          SubmitChangesToScene();
        }
      }
    }
  }
  ImGui::EndChild();
}

void SceneViewEditor::DrawSceneProperties() {
  ImFont* font_awesome = scene()->GetController<ImGuiStartFrameController>()->GetFont("FontAwesomeSolid");

  ImGui::Text("Scene controllers");
  ImGui::SameLine();

  ImVec2 button_position = ImGui::GetCursorScreenPos();
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(48, 48, 48)));
  ImGui::PushFont(font_awesome);
  if (ImGui::Button("\uf0fe")) {
    ImGui::OpenPopup("Add Controller");
  }
  ImGui::PopFont();
  ImGui::PopStyleColor();
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Add Controller");
  }
  
  ImVec2 next_line_pos = ImGui::GetCursorScreenPos();
  ImGui::SetNextWindowPos({button_position.x, next_line_pos.y});
  if (ImGui::BeginPopup("Add Controller")) {
    for (const std::string& controller : SceneController::registered_ids()) {
      if (!game_scene()->HasController(controller)) {
        if (ImGui::Selectable(controller.c_str())) {
          game_scene()->AddController(controller);
          SubmitChangesToScene();
        }
      }
    }
    for (const std::string& controller : GetApplicationAssetLibrary()->GetAssetsWithType("scene_controller")) {
      if (!game_scene()->HasController(controller)) {
        if (ImGui::Selectable(controller.c_str())) {
          game_scene()->AddController(controller);
          SubmitChangesToScene();
        }
      }
    }
    ImGui::EndPopup();
  }

  ImVec2 available_content_region = ImGui::GetContentRegionAvail();
  if (ImGui::BeginChild("SceneControllers", ImVec2(0, available_content_region.y), false)) {
    for (const auto& controller : game_scene()->controllers()) {
      // Get component path
      const json::json_pointer controller_path = GetControllerPath(controller->name());
      if (serialized_scene_editing_copy_.contains(controller_path)) {
        json& serialized_controller = serialized_scene_editing_copy_[controller_path];
        const ovis::json* controller_schema = controller->GetSchema();

        bool keep = true;
        if (ImGui::InputJson(controller->name().c_str(), &serialized_controller,
                            controller_schema ? *controller_schema : ovis::json{}, &keep)) {
          if (!keep) {
            game_scene()->RemoveController(controller->name());
            SubmitChangesToScene();
          } else {
            controller->Deserialize(serialized_controller);
          }
        }
        if (ImGui::IsItemDeactivated()) {
          // After editing is finished reserialize the controller so the input gets "validated"
          SubmitChangesToScene();
        }
      }
    }
  }
  ImGui::EndChild();
}

void SceneViewEditor::SetSerializedScene(const json& data) {
  ChangeRunState(RunState::STOPPED);
  game_scene()->Deserialize(data);
  SubmitJsonFile(data);
  serialized_scene_editing_copy_ = data;
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
        EditorWindow::instance()->context(), scene_viewport_description);

    scene_viewport_->AddRenderPass("ClearPass");
    scene_viewport_->AddRenderPass("SpriteRenderer");
    // scene_viewport_->AddRenderPass("Physics2DDebugLayer");
    scene_viewport_->AddRenderPass(std::make_unique<SelectedObjectBoundingBox>(editing_scene()));
    scene_viewport_->AddRenderPass(std::make_unique<TransformationToolsRenderer>(editing_scene()));
    scene_viewport_->AddRenderPass(std::make_unique<Physics2DShapeRenderer>(editing_scene()));
    // scene_viewport_->AddRenderPassDependency("SpriteRenderer", "Physics2DDebugLayer");
    scene_viewport_->AddRenderPassDependency("SpriteRenderer", SelectedObjectBoundingBox::Name());
    scene_viewport_->AddRenderPassDependency(SelectedObjectBoundingBox::Name(), TransformationToolsRenderer::Name());
    scene_viewport_->AddRenderPassDependency(SelectedObjectBoundingBox::Name(), Physics2DShapeRenderer::Name());
    // scene_viewport_->AddRenderPassDependency("SelectedObjectBoundingBox", "GizmoRenderer");
    // scene_viewport_->AddRenderPassDependency("SpriteRenderer", "GizmoRenderer");
    scene_viewport_->SetScene(game_scene());
    game_scene()->SetMainViewport(scene_viewport_.get());
    editing_scene()->SetMainViewport(scene_viewport_.get());
  } else {
    if (size.x > 0 && size.y > 0) {
      scene_viewport_->Resize(size.x, size.y);
    }
  }
}

SceneObject* SceneViewEditor::CreateObject(const std::string& base_name, bool initiate_rename) {
  std::string object_name = base_name;
  int counter = 1;
  while (game_scene()->ContainsObject(object_name)) {
    counter++;
    object_name = base_name + std::to_string(counter);
  }
  editing_scene()->GetController<ObjectSelectionController>()->SelectObject(object_name);
  if (initiate_rename) {
    renaming_state_ = RenamingState::STARTED_RENAMING;
  }
  return game_scene()->CreateObject(object_name);
}

}  // namespace editor
}  // namespace ovis