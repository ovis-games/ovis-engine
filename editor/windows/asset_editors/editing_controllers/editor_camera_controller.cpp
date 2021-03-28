#include "editor_camera_controller.hpp"

#include <ovis/utils/log.hpp>
#include <ovis/input/mouse_button.hpp>
#include <ovis/input/mouse_events.hpp>

namespace ovis {
namespace editor {

EditorCameraController::EditorCameraController(RenderingViewport* viewport)
    : SceneController("EditorCameraController"), viewport_(viewport) {
  SubscribeToEvent(MouseMoveEvent::TYPE);
  SubscribeToEvent(MouseButtonPressEvent::TYPE);
  SubscribeToEvent(MouseWheelEvent::TYPE);

  camera_.SetProjectionType(ProjectionType::ORTHOGRAPHIC);
  camera_.SetVerticalFieldOfView(100.0f);
  camera_.SetNearClipPlane(0.0f);
}

void EditorCameraController::Update(std::chrono::microseconds delta_time) {
  camera_.SetAspectRatio(viewport_->GetAspectRatio());
  viewport_->SetCustomCameraMatrices(transform_.world_to_local_matrix(), camera_.projection_matrix());
  viewport_->SetCamera(nullptr);
}

void EditorCameraController::ProcessEvent(Event* event) {
  if (event->type() == MouseButtonPressEvent::TYPE) {
    if (static_cast<MouseButtonPressEvent*>(event)->button() == MouseButton::Right()) {
      event->StopPropagation();
    }
  } else if (event->type() == MouseMoveEvent::TYPE) {
    if (GetMouseButtonState(MouseButton::Right())) {
      MouseMoveEvent* mouse_move_event = static_cast<MouseMoveEvent*>(event);

      SDL_assert(viewport_ == mouse_move_event->viewport());
      const Vector3 camera_offset = -viewport_->ScreenSpaceDirectionToWorldSpace(
          Vector3::FromVector2(mouse_move_event->relative_screen_space_position()));
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
