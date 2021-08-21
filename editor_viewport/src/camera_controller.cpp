#include <ovis/editor_viewport/camera_controller.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/input/mouse_button.hpp>
#include <ovis/input/mouse_events.hpp>

#include <ovis/editor_viewport/editor_viewport.hpp>


namespace ovis {
namespace editor {

CameraController::CameraController(EditorViewport* viewport)
    : ViewportController(), camera_(nullptr), viewport_(viewport) {
  // SubscribeToEvent(MouseMoveEvent::TYPE);
  // SubscribeToEvent(MouseButtonPressEvent::TYPE);
  // SubscribeToEvent(MouseWheelEvent::TYPE);

  camera_.SetProjectionType(ProjectionType::ORTHOGRAPHIC);
  camera_.SetVerticalFieldOfView(100.0f);
  camera_.SetNearClipPlane(0.0f);
  camera_position_ = Vector3::Zero();
}

void CameraController::Update(std::chrono::microseconds delta_time) {
  camera_.SetAspectRatio(viewport_->GetAspectRatio());
  viewport_->SetCustomCameraMatrices(Matrix3x4::FromTranslation(-camera_position_), camera_.projection_matrix());
}

void CameraController::ProcessEvent(Event* event) {
  if (event->type() == MouseButtonPressEvent::TYPE) {
    if (static_cast<MouseButtonPressEvent*>(event)->button() == MouseButton::Right()) {
      event->StopPropagation();
    }
  } else if (event->type() == MouseMoveEvent::TYPE) {
    if (GetMouseButtonState(MouseButton::Right())) {
      MouseMoveEvent* mouse_move_event = static_cast<MouseMoveEvent*>(event);
      SDL_assert(viewport_ == mouse_move_event->viewport());

      const Vector2 old_mouse_position =
          mouse_move_event->screen_space_position() - mouse_move_event->relative_screen_space_position();
      const Vector3 previous_world_space_position =
          mouse_move_event->viewport()->ScreenSpacePositionToWorldSpace(Vector3::FromVector2(old_mouse_position, 0.0f));
      const Vector3 new_world_space_position = mouse_move_event->viewport()->ScreenSpacePositionToWorldSpace(
          Vector3::FromVector2(mouse_move_event->screen_space_position(), 0.0f));

      const Vector3 camera_offset = previous_world_space_position - new_world_space_position;

      camera_position_ += camera_offset;
      event->StopPropagation();
    }
  } else if (event->type() == MouseWheelEvent::TYPE) {
    MouseWheelEvent* mouse_wheel_event = static_cast<MouseWheelEvent*>(event);
    camera_.SetVerticalFieldOfView(camera_.vertical_field_of_view() * std::pow(2.0, -mouse_wheel_event->delta().y));
    event->StopPropagation();
  }
}

}  // namespace editor

}  // namespace ovis
