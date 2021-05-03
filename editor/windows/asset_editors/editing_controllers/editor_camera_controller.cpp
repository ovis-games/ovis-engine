#include "editor_camera_controller.hpp"

#include <ovis/utils/log.hpp>
#include <ovis/input/mouse_button.hpp>
#include <ovis/input/mouse_events.hpp>

namespace ovis {
namespace editor {

EditorCameraController::EditorCameraController() : EditorController("EditorCameraController"), camera_(nullptr), transform_(nullptr) {
  SubscribeToEvent(MouseMoveEvent::TYPE);
  SubscribeToEvent(MouseButtonPressEvent::TYPE);
  SubscribeToEvent(MouseWheelEvent::TYPE);

  camera_.SetProjectionType(ProjectionType::ORTHOGRAPHIC);
  camera_.SetVerticalFieldOfView(100.0f);
  camera_.SetNearClipPlane(0.0f);
}

void EditorCameraController::Update(std::chrono::microseconds delta_time) {
  SDL_assert(editing_scene() != nullptr);

  auto viewport = editing_scene()->main_viewport();
  if (!viewport) {
    return;
  }

  camera_.SetAspectRatio(viewport->GetAspectRatio());
  viewport->SetCustomCameraMatrices(transform_.world_to_local_matrix(), camera_.projection_matrix());
}

void EditorCameraController::ProcessEvent(Event* event) {
  if (event->type() == MouseButtonPressEvent::TYPE) {
    if (static_cast<MouseButtonPressEvent*>(event)->button() == MouseButton::Right()) {
      event->StopPropagation();
    }
  } else if (event->type() == MouseMoveEvent::TYPE) {
    if (GetMouseButtonState(MouseButton::Right())) {
      MouseMoveEvent* mouse_move_event = static_cast<MouseMoveEvent*>(event);
      SDL_assert(scene()->main_viewport() == mouse_move_event->viewport());

      const Vector2 old_mouse_position =
          mouse_move_event->screen_space_position() - mouse_move_event->relative_screen_space_position();
      const Vector3 previous_world_space_position =
          mouse_move_event->viewport()->ScreenSpacePositionToWorldSpace(Vector3::FromVector2(old_mouse_position, 0.0f));
      const Vector3 new_world_space_position = mouse_move_event->viewport()->ScreenSpacePositionToWorldSpace(
          Vector3::FromVector2(mouse_move_event->screen_space_position(), 0.0f));

      const Vector3 camera_offset = previous_world_space_position - new_world_space_position;

      transform_.Move(camera_offset);
      event->StopPropagation();
    }
  } else if (event->type() == MouseWheelEvent::TYPE) {
    MouseWheelEvent* mouse_wheel_event = static_cast<MouseWheelEvent*>(event);
    camera_.SetVerticalFieldOfView(camera_.vertical_field_of_view() * std::powf(2.0, -mouse_wheel_event->delta().y));
    event->StopPropagation();
  }
}

}  // namespace editor

}  // namespace ovis
