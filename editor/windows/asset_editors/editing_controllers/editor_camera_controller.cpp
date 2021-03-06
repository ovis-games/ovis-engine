#include "editor_camera_controller.hpp"

#include <ovis/engine/input.hpp>

namespace ovis {
namespace editor {

EditorCameraController::EditorCameraController(Viewport* viewport)
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
  viewport_->SetCamera(camera_, transform_.CalculateMatrix());
}

void EditorCameraController::ProcessEvent(Event* event) {
  if (event->type() == MouseButtonPressEvent::TYPE) {
    if (static_cast<MouseButtonPressEvent*>(event)->button() == MouseButton::RIGHT) {
      right_button_down_ = true;
      event->StopPropagation();
    }
  } else if (event->type() == MouseMoveEvent::TYPE) {
    if (!input()->GetMouseButtonState(MouseButton::RIGHT)) {
      right_button_down_ = false;
    }
    if (right_button_down_) {
      MouseMoveEvent* mouse_move_event = static_cast<MouseMoveEvent*>(event);

      SDL_assert(viewport_ == mouse_move_event->viewport());
      const Vector3 world0 = viewport_->DeviceCoordinatesToWorldSpace(Vector3::Zero());
      const Vector3 world1 = viewport_->DeviceCoordinatesToWorldSpace(mouse_move_event->relative_device_coordinates());
      transform_.Move(world0 - world1);
      event->StopPropagation();
    }
  } else if (event->type() == MouseWheelEvent::TYPE) {
    MouseWheelEvent* mouse_wheel_event = static_cast<MouseWheelEvent*>(event);
    camera_.SetVerticalFieldOfView(camera_.vertical_field_of_view() * std::powf(2.0, -mouse_wheel_event->delta_y()));
    event->StopPropagation();
  }
}

}  // namespace editor

}  // namespace ovis
