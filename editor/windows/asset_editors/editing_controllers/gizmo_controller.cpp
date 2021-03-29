#include "gizmo_controller.hpp"

#include "object_selection_controller.hpp"

#include <ovis/utils/log.hpp>
#include <ovis/core/scene_viewport.hpp>
#include <ovis/core/transform.hpp>
#include <ovis/input/mouse_button.hpp>
#include <ovis/input/mouse_events.hpp>

namespace ovis {
namespace editor {

float DistanceToLineSegment(Vector2 point, const LineSegment2D& line_segment) {
  // https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
  const Vector2 line_segment_vector = line_segment.endpoints[1] - line_segment.endpoints[0];
  const float squared_line_segment_length = SquaredLength(line_segment_vector);
  if (squared_line_segment_length == 0.0f) {
    return Length(point - line_segment.endpoints[0]);
  }

  const float t =
      clamp(Dot(point - line_segment.endpoints[0], line_segment_vector) / squared_line_segment_length, 0.0f, 1.0f);
  const Vector2 projected_point_on_line_segment = line_segment.endpoints[0] + t * line_segment_vector;
  return Length(point - projected_point_on_line_segment);
}

GizmoController::GizmoController(Scene* game_scene) : SceneController("GizmoController"), game_scene_(game_scene) {
  SubscribeToEvent(MouseMoveEvent::TYPE);
  SubscribeToEvent(MouseButtonPressEvent::TYPE);
  SubscribeToEvent(MouseButtonReleaseEvent::TYPE);
}

void GizmoController::Update(std::chrono::microseconds) {
  auto* object_selection_controller = scene()->GetController<ObjectSelectionController>("ObjectSelectionController");
  SDL_assert(object_selection_controller != nullptr);

  SceneObject* scene_object = object_selection_controller->selected_object();
  if (!scene_object) {
    object_selected_ = false;
    return;
  }
  object_selected_ = true;

  Transform* transform = scene_object->GetComponent<Transform>("Transform");
  if (transform != nullptr) {
    auto viewport = game_scene_->main_viewport();
    object_position_screen_space_ = viewport->WorldSpacePositionToScreenSpace(transform->position());

    Vector2 position2d = object_position_screen_space_;

    const Quaternion rotation = transform->rotation();

    const Vector3 unit_offset = viewport->WorldSpacePositionToScreenSpace(transform->position() + Vector3::PositiveX());
    const float scaling_factor = gizmo_radius_ / Length(unit_offset - object_position_screen_space_);

    auto get_endpoint_in_screen_space = [&, this](Vector3 axis) -> Vector2 {
      return viewport->WorldSpacePositionToScreenSpace(transform->position() + scaling_factor * (rotation * axis));
    };

    line_x_ = {position2d, get_endpoint_in_screen_space(Vector3::PositiveX())};
    line_y_ = {position2d, get_endpoint_in_screen_space(Vector3::PositiveY())};
    line_z_ = {position2d, get_endpoint_in_screen_space(Vector3::PositiveZ())};
  }
}

void GizmoController::ProcessEvent(Event* event) {
  if (!object_selected_) {
    return;
  }

  auto* object_selection_controller = scene()->GetController<ObjectSelectionController>("ObjectSelectionController");
  SDL_assert(object_selection_controller != nullptr);
  SceneObject* scene_object = object_selection_controller->selected_object();
  SDL_assert(scene_object);
  Transform* transform = scene_object->GetComponent<Transform>("Transform");
  SDL_assert(transform);

  if (event->type() == MouseMoveEvent::TYPE) {
    auto mouse_move_event = static_cast<MouseMoveEvent*>(event);
    const Vector2 mouse_position = mouse_move_event->screen_space_position();

    if (!GetMouseButtonState(MouseButton::Left())) {
      if (CheckMousePosition(mouse_position)) {
        event->StopPropagation();
      }
    } else {
      Vector2 direction = Vector3::Zero();
      if (movement_selection_ == MovementSelection::X) {
        direction = line_x_.endpoints[1] - line_x_.endpoints[0];
      } else if (movement_selection_ == MovementSelection::Y) {
        direction = line_y_.endpoints[1] - line_y_.endpoints[0];
      } else if (movement_selection_ == MovementSelection::Z) {
        direction = line_z_.endpoints[1] - line_z_.endpoints[0];
      }

      Vector2 screen_space_offset = Vector3::Zero();
      if (SquaredLength(direction) > 0) {
        const Vector2 normalized_direction = Normalize(direction);
        const float t = Dot(normalized_direction, mouse_move_event->relative_screen_space_position());
        screen_space_offset = t * normalized_direction;
      } else if (movement_selection_ != MovementSelection::NOTHING) {
        screen_space_offset = mouse_move_event->relative_screen_space_position();
      }

      const Vector2 old_mouse_position = mouse_position - mouse_move_event->relative_screen_space_position();
      const Vector3 previous_world_space_position = mouse_move_event->viewport()->ScreenSpacePositionToWorldSpace(
          Vector3::FromVector2(old_mouse_position, object_position_screen_space_.z));
      const Vector3 new_world_space_position = mouse_move_event->viewport()->ScreenSpacePositionToWorldSpace(
          Vector3::FromVector2(old_mouse_position + screen_space_offset, object_position_screen_space_.z));

      const Vector3 world_space_offset = new_world_space_position - previous_world_space_position;
      transform->Move(world_space_offset);
    }
  } else if (event->type() == MouseButtonPressEvent::TYPE) {
    auto button_press_event = static_cast<MouseButtonPressEvent*>(event);
    const Vector2 position = button_press_event->screen_space_position();
    if (CheckMousePosition(position)) {
      event->StopPropagation();
    }
  } else if (event->type() == MouseButtonReleaseEvent::TYPE) {
    auto button_release_event = static_cast<MouseButtonReleaseEvent*>(event);
    if (movement_selection_ != MovementSelection::NOTHING) {
      event->StopPropagation();
    }
    CheckMousePosition(button_release_event->screen_space_position());
  }
}

bool GizmoController::CheckMousePosition(Vector2 position) {
  if (Length(position - static_cast<Vector2>(object_position_screen_space_)) <= point_size_) {
    movement_selection_ = MovementSelection::XYZ;
    return true;
  } else if (DistanceToLineSegment(position, line_x_) <= line_thickness_) {
    movement_selection_ = MovementSelection::X;
    return true;
  } else if (DistanceToLineSegment(position, line_y_) <= line_thickness_) {
    movement_selection_ = MovementSelection::Y;
    return true;
  } else if (DistanceToLineSegment(position, line_z_) <= line_thickness_) {
    movement_selection_ = MovementSelection::Z;
    return true;
  } else {
    movement_selection_ = MovementSelection::NOTHING;
    return false;
  }
}

}  // namespace editor
}  // namespace ovis
