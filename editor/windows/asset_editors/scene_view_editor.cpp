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

#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <ovis/utils/utf8.hpp>
#include <ovis/utils/utils.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/scene_controller.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/transform.hpp>
#include <ovis/rendering2d/sprite.hpp>
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

SceneViewEditor::SceneViewEditor(const std::string& scene_asset, bool allow_adding_objects)
  : AssetEditor(scene_asset), allow_adding_objects_(allow_adding_objects) {
  editing_scene()->Play();
  AddEditorController<ObjectSelectionController>();
  AddEditorController<TransformationToolsController>();
  AddEditorController<EditorCameraController>();
  AddEditorController<Physics2DShapeController>();

  SubscribeToEvent(LuaErrorEvent::TYPE);
  CreateSceneViewports({1, 1});
}

void SceneViewEditor::Update(std::chrono::microseconds delta_time) {
  if (run_state() == RunState::RUNNING) {
    game_scene_.Update(delta_time);
    serialized_scene_editing_copy_ = game_scene_.Serialize();
  } else {
    editing_scene_.Update(delta_time);
  }
  AssetEditor::Update(delta_time);
  for (const auto& controller : editing_scene_.controllers()) {
    if (auto editor_controller = dynamic_cast<const EditorController*>(controller.get()); editor_controller != nullptr) {
      if (editor_controller->has_tooltip()) {
        ImGui::SetTooltip("%s", editor_controller->tooltip());
      }
    }
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
        SubmitChanges();
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

void SceneViewEditor::SelectObject(SceneObject* object) {
  auto* object_selection_controller = editing_scene()->GetController<ObjectSelectionController>();
  SDL_assert(object_selection_controller != nullptr);
  object_selection_controller->SelectObject(object);
  SDL_assert(object_selection_controller->selected_object());
  SDL_assert(object_selection_controller->selected_object() == object);
  SDL_assert(object_selection_controller->selected_object()->path() == object->path());
}


SceneObject* SceneViewEditor::GetSelectedObject() {
  auto* object_selection_controller = editing_scene()->GetController<ObjectSelectionController>();
  SDL_assert(object_selection_controller != nullptr);
  return object_selection_controller->selected_object();
}

void SceneViewEditor::UpdateSceneEditingCopy() {
  serialized_scene_editing_copy_ = game_scene()->Serialize();
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
    SelectObject(object);
    SubmitChanges();
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

  if (!game_viewport_ || !editor_viewport_ ||
      ((Vector2{available_space.x, available_space.y} != game_viewport_->GetDimensions() ||
        Vector2{available_space.x, available_space.y} != editor_viewport_->GetDimensions())&&
       !ImGui::IsMouseDown(ImGuiMouseButton_Left))) {  // TODO: this check should be optimizied. Dragging can mean a
                                                       // lot of things not necessarily resizing the window
    LogI("Creating viewport");
    CreateSceneViewports(available_space);
  }
  RenderTargetViewport* viewport;
  if (run_state() == RunState::RUNNING) {
    viewport = game_viewport_.get();
  } else {
    viewport = editor_viewport_.get();
  }
  viewport->Render();

  const Vector2 top_left = static_cast<Vector2>(ImGui::GetWindowPos()) + static_cast<Vector2>(ImGui::GetCursorPos());
  ImGui::Image(viewport->color_texture()->texture()->id(), available_space, ImVec2(0, 1), ImVec2(1, 0),
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
      const Vector3 screen_space_position = viewport->WorldSpacePositionToScreenSpace(transform->world_position());
      const Vector3 new_screen_space_position = Vector3::FromVector2(mouse_position, screen_space_position.z);
      const Vector3 new_world_space_position = viewport->ScreenSpacePositionToWorldSpace(new_screen_space_position);
      transform->SetWorldPosition(new_world_space_position);
      SubmitChanges();
    }
  }
  if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete))) {
    LogI("Delete object");
    game_scene()->DeleteObject(GetSelectedObject());
    SubmitChanges();
  }

  if (ImGui::IsItemHovered()) {
    if (io.MouseWheel != 0 || io.MouseWheelH != 0) {
      MouseWheelEvent mouse_wheel_event({io.MouseWheelH, io.MouseWheel});
      ProcessViewportInputEvent(&mouse_wheel_event);
    }
    if (mouse_position != latest_mouse_position_) {
      const Vector2 relative_position =
          std::isnan(latest_mouse_position_.x) || std::isnan(latest_mouse_position_.y) ? Vector2::Zero() : mouse_position - latest_mouse_position_;
      MouseMoveEvent mouse_move_event(viewport, mouse_position, relative_position);
      ProcessViewportInputEvent(&mouse_move_event);
      latest_mouse_position_ = mouse_position;
    }
    for (int i = 0; i < 5; ++i) {
      if (ImGui::IsMouseClicked(i)) {
        MouseButtonPressEvent mouse_button_event(viewport, mouse_position, GetMouseButtonFromImGuiIndex(i));
        ProcessViewportInputEvent(&mouse_button_event);
      }
      if (ImGui::IsMouseReleased(i)) {
        MouseButtonReleaseEvent mouse_button_event(viewport, mouse_position, GetMouseButtonFromImGuiIndex(i));
        ProcessViewportInputEvent(&mouse_button_event);
      }
    }
  } else {
    latest_mouse_position_ = Vector2::NotANumber();
  }

  if (allow_adding_objects_ && ImGui::BeginDragDropTarget()) {
    if (std::string dropped_asset_id; run_state() != RunState::RUNNING && ImGui::AcceptDragDropAsset("texture2d", &dropped_asset_id)) {
      SceneObject* object = game_scene()->CreateObject(dropped_asset_id);

      auto* transform = object->AddComponent<Transform>("Transform");
      const Vector2 mouse_position = ImGui::GetMousePos();
      transform->SetWorldPosition(editor_viewport_->ScreenSpacePositionToWorldSpace(Vector3::FromVector2(mouse_position - top_left)));

      auto* sprite = object->AddComponent<Sprite>("Sprite");
      sprite->SetTexture(dropped_asset_id);
      sprite->SetSize({ 1.0f, 1.0f });

      SubmitChanges();
    }
    if (std::string dropped_asset_id; run_state() != RunState::RUNNING && ImGui::AcceptDragDropAsset("scene_object", &dropped_asset_id)) {
      SceneObject* object = game_scene()->CreateObject(dropped_asset_id);
      if (!object->SetupTemplate(dropped_asset_id)) {
        LogE("Failed to setup template `{}`", dropped_asset_id);
      } else {
        if (auto* transform = object->GetComponent<Transform>("Transform"); transform != nullptr) {
          const Vector2 mouse_position = ImGui::GetMousePos();
          transform->SetWorldPosition(editor_viewport_->ScreenSpacePositionToWorldSpace(Vector3::FromVector2(mouse_position - top_left)));
        }

        SubmitChanges();
      }
    }

    ImGui::EndDragDropTarget();
  }
}

void SceneViewEditor::DrawInspectorContent() {
  ImVec2 available_content_region = ImGui::GetContentRegionAvail();
  if (ImGui::BeginChild("ObjectView", ImVec2(0, available_content_region.y / 2), false)) {
    DrawObjectTree();
  }
  ImGui::EndChild();
  ImGui::Separator();
  DrawSelectionProperties();
}

void SceneViewEditor::DrawObjectHierarchy(SceneObject* object) {
  ImGuiIO& io = ImGui::GetIO();
  const bool control_or_command = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
  const bool is_object_selected = GetSelectedObject() == object;

  ImGuiTreeNodeFlags scene_object_flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                          ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
  if (!object->HasChildren()) {
    scene_object_flags |= ImGuiTreeNodeFlags_Leaf;
  }
  if (is_object_selected) {
    scene_object_flags |= ImGuiTreeNodeFlags_Selected;
  }

  bool node_open;
  const std::string object_name(object->name());
  if (renaming_state_ == RenamingState::IS_NOT_RENAMING || !is_object_selected) {
    // This object is currently not beeing renamed
    node_open = ImGui::TreeNodeEx(object_name.c_str(), scene_object_flags);
    if (ImGui::IsItemFocused() && control_or_command && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C))) {
      CopySelectedObjectToClipboard();
    }
    if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete))) {
      game_scene()->DeleteObject(GetSelectedObject());
      SubmitChanges();
    }

    if (ImGui::BeginDragDropSource()) {
      static_assert(std::is_same_v<typeof(object), SceneObject*>);
      ImGui::SetDragDropPayload("scene_object", &object, sizeof(object), ImGuiCond_Always);
      ImGui::Text("%s", object_name.c_str());
      ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
      const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_object");
      if (payload) {
        SceneObject* dragged_object = *reinterpret_cast<SceneObject**>(payload->Data);
        if (dragged_object->parent() != object) {
          DoOnceAfterUpdate([this, dragged_object, object]() {
            object->CreateChildObject(dragged_object->name(), dragged_object->Serialize());
            dragged_object->scene()->DeleteObject(dragged_object);
            SubmitChanges();
          });
        }
      }
      ImGui::EndDragDropTarget();
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      SelectObject(object);
    }

    if (ImGui::BeginPopupContextItem()) {
      if (ImGui::Selectable("Create Child Object")) {
        SelectObject(object->CreateChildObject("New Object"));
        SubmitChanges();
        renaming_state_ = RenamingState::STARTED_RENAMING;
      }
      if (ImGui::Selectable("Rename")) {
        renaming_state_ = RenamingState::STARTED_RENAMING;
      }
      if (ImGui::Selectable("Duplicate")) {
        if (object->parent()) {
          object->parent()->CreateChildObject(object->name(), object->Serialize());
        } else {
          game_scene()->CreateObject(object->name(), object->Serialize());
        }
        SubmitChanges();
      }
      if (ImGui::Selectable("Remove")) {
        DoOnceAfterUpdate([=, this] {
            game_scene()->DeleteObject(object->path());
            SubmitChanges();
            });
      }
      ImGui::EndPopup();
    }
  } else {
    // This object is currently beeing renamed
    std::string new_object_name = object_name;

    ::ImGuiWindow* window = ImGui::GetCurrentWindow();
    node_open = ImGui::TreeNodeBehaviorIsOpen(window->GetID(object_name.c_str()), scene_object_flags);
    if (node_open) {
      ImGui::TreePush("object renaming");
    }

    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("Renaming", &new_object_name,
                         ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
      DoOnceAfterUpdate([=, this] {
        if (new_object_name != object->name()) {
          SceneObject* parent = object->parent();
          if (!SceneObject::IsValidName(new_object_name)) {
            EditorWindow::instance()->AddToastNotification(ToastType::ERROR, "Object names may only contain letters, numbers, underscores(_), dashes(-) and periods(.)");
            renaming_state_ = RenamingState::STARTED_RENAMING;
          } else if (parent != nullptr ? parent->ContainsChildObject(new_object_name) : game_scene()->ContainsObject(new_object_name)) {
            EditorWindow::instance()->AddToastNotification(ToastType::ERROR, "The object already exists.");
            renaming_state_ = RenamingState::STARTED_RENAMING;
          } else {
            if (parent != nullptr) {
              SelectObject(parent->CreateChildObject(new_object_name, object->Serialize()));
              parent->DeleteChildObject(object->name());
            } else {
              SelectObject(game_scene()->CreateObject(new_object_name, object->Serialize()));
              game_scene()->DeleteObject(object->name());
            }
            SubmitChanges();
            renaming_state_ = RenamingState::IS_NOT_RENAMING;
          }
        } else {
          renaming_state_ = RenamingState::IS_NOT_RENAMING;
        }
      });
    }
    if (renaming_state_ == RenamingState::STARTED_RENAMING) {
      ImGui::SetKeyboardFocusHere();
      renaming_state_ = RenamingState::IS_RENAMING;
    } else if (!ImGui::IsItemActive()) {
      renaming_state_ = RenamingState::IS_NOT_RENAMING;
    }
    ImGui::PopItemWidth();
  }
  if (node_open) {
    for (safe_ptr<SceneObject>& child : object->children()) {
      DrawObjectHierarchy(child.get());
    }
    ImGui::TreePop();
  }
}

void SceneViewEditor::DrawSelectionProperties() {
  if (GetSelectedObject() != nullptr) {
    DrawSceneObjectProperties();
  } else {
    DrawSceneProperties();
  }
}

void SceneViewEditor::DrawSceneObjectProperties() {
  SceneObject* selected_object = GetSelectedObject();
  SDL_assert(selected_object != nullptr);

  if (!selected_object) {
    return;
  }

  ImFont* font_awesome = scene()->GetController<ImGuiStartFrameController>()->GetFont("FontAwesomeSolid");

  std::string object_name(selected_object->name());
  ImGui::Text("Components of %s:", object_name.c_str());
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
          SubmitChanges();
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
      SDL_assert(GetSelectedObject() != nullptr);
      const json::json_pointer component_path = GetComponentPath(GetSelectedObject()->path(), component_id);
      if (serialized_scene_editing_copy_.contains(component_path)) {
        json& serialized_component = serialized_scene_editing_copy_[component_path];
        const ovis::json* component_schema = component->GetSchema();

        bool keep = true;
        if (ImGui::InputJson(component_id.c_str(), &serialized_component,
                            component_schema ? *component_schema : ovis::json{}, &keep)) {
          if (!keep) {
            selected_object->RemoveComponent(component_id);
            SubmitChanges();
          } else {
            component->Deserialize(serialized_component);
            if (!ImGui::IsItemActive()) {
              // Only submit the changes if the item is currently not active.
              // Items are active, e.g., while "dragging" a DragFloat(). We do
              // not want to have undo steps each frame but only after the
              // dragging is finished. I.e., when the item is deactivated (see
              // below).
              SubmitChanges();
            }
          }
        }
        if (ImGui::IsItemDeactivated()) {
          // After editing is finished reserialize the component so the input gets "validated"
          // and an undo step is created.
          SubmitChanges();
        }
      }
    }
  }
  ImGui::EndChild();
}

void SceneViewEditor::DrawSceneProperties() {
  ImFont* font_awesome = scene()->GetController<ImGuiStartFrameController>()->GetFont("FontAwesomeSolid");

  std::string camera = serialized_scene_editing_copy_.contains("camera") ? serialized_scene_editing_copy_.at("camera") : "";
  if (ImGui::InputText("Camera", &camera, ImGuiInputTextFlags_EnterReturnsTrue)) {
    serialized_scene_editing_copy_["camera"] = camera;
    game_scene()->Deserialize(serialized_scene_editing_copy_);
    SubmitChanges();
  }

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
          SubmitChanges();
        }
      }
    }
    for (const std::string& controller : GetApplicationAssetLibrary()->GetAssetsWithType("scene_controller")) {
      if (!game_scene()->HasController(controller)) {
        if (ImGui::Selectable(controller.c_str())) {
          game_scene()->AddController(controller);
          SubmitChanges();
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
            SubmitChanges();
          } else {
            controller->Deserialize(serialized_controller);
          }
        }
        if (ImGui::IsItemDeactivated()) {
          // After editing is finished reserialize the controller so the input gets "validated"
          SubmitChanges();
        }
      }
    }
  }
  ImGui::EndChild();
}

void SceneViewEditor::CreateSceneViewports(ImVec2 size) {
  if (!editor_viewport_) {
    RenderTargetViewportDescription editor_viewport_description;
    editor_viewport_description.color_description.texture_description.width = static_cast<size_t>(size.x);
    editor_viewport_description.color_description.texture_description.height = static_cast<size_t>(size.y);
    editor_viewport_description.color_description.texture_description.format = TextureFormat::RGB_UINT8;
    editor_viewport_description.color_description.texture_description.filter = TextureFilter::POINT;
    editor_viewport_description.color_description.texture_description.mip_map_count = 0;
    editor_viewport_ = std::make_unique<RenderTargetViewport>(
        EditorWindow::instance()->context(), editor_viewport_description);

    editor_viewport_->AddRenderPass("ClearPass");
    editor_viewport_->AddRenderPass("SpriteRenderer");
    // editor_viewport_->AddRenderPass("Physics2DDebugLayer");
    editor_viewport_->AddRenderPass(std::make_unique<SelectedObjectBoundingBox>(editing_scene()));
    editor_viewport_->AddRenderPass(std::make_unique<TransformationToolsRenderer>(editing_scene()));
    editor_viewport_->AddRenderPass(std::make_unique<Physics2DShapeRenderer>(editing_scene()));
    // editor_viewport_->AddRenderPassDependency("SpriteRenderer", "Physics2DDebugLayer");
    editor_viewport_->AddRenderPassDependency("SpriteRenderer", SelectedObjectBoundingBox::Name());
    editor_viewport_->AddRenderPassDependency(SelectedObjectBoundingBox::Name(), TransformationToolsRenderer::Name());
    editor_viewport_->AddRenderPassDependency(SelectedObjectBoundingBox::Name(), Physics2DShapeRenderer::Name());
    // editor_viewport_->AddRenderPassDependency("SelectedObjectBoundingBox", "GizmoRenderer");
    // editor_viewport_->AddRenderPassDependency("SpriteRenderer", "GizmoRenderer");
    editor_viewport_->SetScene(game_scene());
    editing_scene()->SetMainViewport(editor_viewport_.get());

    RenderTargetViewportDescription game_viewport_description;
    game_viewport_description.color_description.texture_description.width = static_cast<size_t>(size.x);
    game_viewport_description.color_description.texture_description.height = static_cast<size_t>(size.y);
    game_viewport_description.color_description.texture_description.format = TextureFormat::RGB_UINT8;
    game_viewport_description.color_description.texture_description.filter = TextureFilter::POINT;
    game_viewport_description.color_description.texture_description.mip_map_count = 0;
    game_viewport_ = std::make_unique<RenderTargetViewport>(
        EditorWindow::instance()->context(), game_viewport_description);

    game_viewport_->AddRenderPass("ClearPass");
    game_viewport_->AddRenderPass("SpriteRenderer");
    game_viewport_->SetScene(game_scene());
    game_scene()->SetMainViewport(game_viewport_.get());
  } else {
    if (size.x > 0 && size.y > 0) {
      editor_viewport_->Resize(size.x, size.y);
      game_viewport_->Resize(size.x, size.y);
    }
  }
}

json::json_pointer SceneViewEditor::GetComponentPath(std::string_view object_path, std::string_view component_id) {
  return json::json_pointer(
      fmt::format("/objects/{}/components/{}", replace_all(object_path, "/", "/children/"), component_id));
}

}  // namespace editor
}  // namespace ovis
