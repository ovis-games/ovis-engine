#include "transformation_tools_controller.hpp"

#include "object_selection_controller.hpp"

#include <imgui.h>

#include <ovis/utils/log.hpp>
#include <ovis/core/intersection.hpp>
#include <ovis/core/scene_viewport.hpp>
#include <ovis/core/transform.hpp>
#include <ovis/input/key_events.hpp>
#include <ovis/input/mouse_button.hpp>

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

TransformationToolsController::TransformationToolsController() : EditorController(Name()) {
  SubscribeToEvent(MouseMoveEvent::TYPE);
  SubscribeToEvent(MouseButtonPressEvent::TYPE);
  SubscribeToEvent(MouseButtonReleaseEvent::TYPE);
}

void TransformationToolsController::Update(std::chrono::microseconds) {
  auto* object_selection_controller = scene()->GetController<ObjectSelectionController>("ObjectSelectionController");
  SDL_assert(object_selection_controller != nullptr);

  SceneObject* scene_object = object_selection_controller->selected_object();
  if (!scene_object) {
    object_selected_ = false;
    return;
  }

  Transform* transform = scene_object->GetComponent<Transform>("Transform");
  if (!transform) {
    object_selected_ = false;
    return;
  }
  object_selected_ = true;

  auto viewport = game_scene()->main_viewport();
  object_position_world_space_ = transform->position();
  object_position_screen_space_ = viewport->WorldSpacePositionToScreenSpace(object_position_world_space_);

  Vector2 position2d = object_position_screen_space_;

  const Quaternion rotation = transform->rotation();

  const Vector3 unit_offset = viewport->WorldSpacePositionToScreenSpace(transform->position() + Vector3::PositiveX());
  world_to_screen_space_factor_ = Distance(unit_offset, object_position_screen_space_);

  // Scaling in world space is not possible
  if (type_ == TransformationType::SCALE && coordinate_system_ == CoordinateSystem::WORLD) {
    coordinate_system_ = CoordinateSystem::OBJECT;
  }

  switch (coordinate_system_) {
    case CoordinateSystem::OBJECT:
      x_axis_world_space_ =
          (gizmo_radius_screen_space_ / world_to_screen_space_factor_) * (rotation * Vector3::PositiveX());
      y_axis_world_space_ =
          (gizmo_radius_screen_space_ / world_to_screen_space_factor_) * (rotation * Vector3::PositiveY());
      z_axis_world_space_ =
          (gizmo_radius_screen_space_ / world_to_screen_space_factor_) * (rotation * Vector3::PositiveZ());
      break;

    case CoordinateSystem::WORLD:
      x_axis_world_space_ = (gizmo_radius_screen_space_ / world_to_screen_space_factor_) * Vector3::PositiveX();
      y_axis_world_space_ = (gizmo_radius_screen_space_ / world_to_screen_space_factor_) * Vector3::PositiveY();
      z_axis_world_space_ = (gizmo_radius_screen_space_ / world_to_screen_space_factor_) * Vector3::PositiveZ();
      break;
  }

  x_axis_endpoint_screen_space_ =
      viewport->WorldSpacePositionToScreenSpace(object_position_world_space_ + x_axis_world_space_);
  y_axis_endpoint_screen_space_ =
      viewport->WorldSpacePositionToScreenSpace(object_position_world_space_ + y_axis_world_space_);
  z_axis_endpoint_screen_space_ =
      viewport->WorldSpacePositionToScreenSpace(object_position_world_space_ + z_axis_world_space_);

  if (current_tooltip_.length() != 0) {
    ImGui::SetTooltip("%s", current_tooltip_.c_str());
  }
}

void TransformationToolsController::ProcessEvent(Event* event) {
  if (!object_selected_) {
    return;
  }

  if (event->type() == MouseMoveEvent::TYPE) {
    auto mouse_move_event = static_cast<MouseMoveEvent*>(event);

    if (!GetMouseButtonState(MouseButton::Left())) {
      if (CheckMousePosition(mouse_move_event->screen_space_position())) {
        event->StopPropagation();
      }
    } else {
      if (selected_axes_ != AxesSelection::NONE) {
        HandleDragging(mouse_move_event);
      }
    }
  } else if (event->type() == MouseButtonPressEvent::TYPE) {
    auto button_press_event = static_cast<MouseButtonPressEvent*>(event);
    const Vector2 position = button_press_event->screen_space_position();
    if (CheckMousePosition(position)) {
      event->StopPropagation();
    }
  } else if (event->type() == MouseButtonReleaseEvent::TYPE) {
    auto button_release_event = static_cast<MouseButtonReleaseEvent*>(event);
    current_tooltip_ = "";
    if (selected_axes_ != AxesSelection::NONE) {
      SubmitChangesToScene();
      event->StopPropagation();
    }
    // We may hover another gizmo now, so recheck it
    CheckMousePosition(button_release_event->screen_space_position());
  }
}

bool TransformationToolsController::CheckMousePosition(Vector2 position) {
  switch (transformation_type()) {
    case TransformationToolsController::TransformationType::MOVE:
      [[fallthrough]];
    case TransformationType::SCALE:
      if (Length(position - static_cast<Vector2>(object_position_screen_space_)) <= point_size_screen_space_) {
        selected_axes_ = AxesSelection::XYZ;
        return true;
      } else if (DistanceToLineSegment(position, {object_position_screen_space_, x_axis_endpoint_screen_space_}) <=
                 line_thickness_screen_space_) {
        selected_axes_ = AxesSelection::X;
        return true;
      } else if (DistanceToLineSegment(position, {object_position_screen_space_, y_axis_endpoint_screen_space_}) <=
                 line_thickness_screen_space_) {
        selected_axes_ = AxesSelection::Y;
        return true;
      } else if (DistanceToLineSegment(position, {object_position_screen_space_, z_axis_endpoint_screen_space_}) <=
                 line_thickness_screen_space_) {
        selected_axes_ = AxesSelection::Z;
        return true;
      } else {
        selected_axes_ = AxesSelection::NONE;
        return false;
      }

    case TransformationType::ROTATE:
      auto viewport = game_scene()->main_viewport();
      const Ray3D view_ray = viewport->CalculateViewRay(position);
      auto hitting_rotate_gizmo = [this, view_ray, viewport](Vector3 rotation_axis) -> bool {
        const Plane3D plane = Plane3D::FromPointAndNormal(object_position_world_space_, rotation_axis);
        auto intersection = ComputeRayPlaneIntersection(view_ray, plane);

        if (!intersection) {
          return false;
        }

        const float distance = Distance(*intersection, object_position_world_space_);
        return world_to_screen_space_factor_ *
                   std::abs(distance - gizmo_radius_screen_space_ / world_to_screen_space_factor_) <=
               line_thickness_screen_space_;
      };

      if (hitting_rotate_gizmo(x_axis_world_space_)) {
        selected_axes_ = AxesSelection::X;
        return true;
      } else if (hitting_rotate_gizmo(y_axis_world_space_)) {
        selected_axes_ = AxesSelection::Y;
        return true;
      } else if (hitting_rotate_gizmo(z_axis_world_space_)) {
        selected_axes_ = AxesSelection::Z;
        return true;
      } else {
        selected_axes_ = AxesSelection::NONE;
        return false;
      }
  }
}

void TransformationToolsController::HandleDragging(MouseMoveEvent* mouse_move_event) {
  auto* object_selection_controller = scene()->GetController<ObjectSelectionController>("ObjectSelectionController");
  SDL_assert(object_selection_controller != nullptr);
  SceneObject* scene_object = object_selection_controller->selected_object();
  SDL_assert(scene_object);
  Transform* transform = scene_object->GetComponent<Transform>("Transform");
  SDL_assert(transform);

  const Vector2 current_mouse_position_screen_space = mouse_move_event->screen_space_position();
  const Vector2 previous_mouse_position_screen_space =
      current_mouse_position_screen_space - mouse_move_event->relative_screen_space_position();

  switch (transformation_type()) {
    case TransformationToolsController::TransformationType::MOVE: {
      Vector3 world_space_direction = Vector3::Zero();
      if (selected_axes_ == AxesSelection::X) {
        world_space_direction = x_axis_world_space_;
      } else if (selected_axes_ == AxesSelection::Y) {
        world_space_direction = y_axis_world_space_;
      } else if (selected_axes_ == AxesSelection::Z) {
        world_space_direction = z_axis_world_space_;
      }

      const Vector3 previous_mouse_world_space_position = mouse_move_event->viewport()->ScreenSpacePositionToWorldSpace(
          Vector3::FromVector2(previous_mouse_position_screen_space, object_position_screen_space_.z));
      const Vector3 current_mouse_world_space_position = mouse_move_event->viewport()->ScreenSpacePositionToWorldSpace(
          Vector3::FromVector2(current_mouse_position_screen_space, object_position_screen_space_.z));
      const Vector3 world_space_offset = current_mouse_world_space_position - previous_mouse_world_space_position;

      Vector2 screen_space_offset = Vector3::Zero();
      if (SquaredLength(world_space_direction) > 0) {
        const Vector3 normalized_world_space_direction = Normalize(world_space_direction);
        const float t = Dot(normalized_world_space_direction, world_space_offset);
        transform->Move(t * normalized_world_space_direction);
      } else if (selected_axes_ != AxesSelection::NONE) {
        transform->Move(world_space_offset);
      }
      current_tooltip_ = fmt::format("{}", transform->position());
      break;
    }

    case TransformationType::ROTATE: {
      Vector3 rotation_axis;
      if (selected_axes_ == AxesSelection::X) {
        rotation_axis = x_axis_world_space_;
      } else if (selected_axes_ == AxesSelection::Y) {
        rotation_axis = y_axis_world_space_;
      } else if (selected_axes_ == AxesSelection::Z) {
        rotation_axis = z_axis_world_space_;
      }

      const Plane3D plane = Plane3D::FromPointAndNormal(object_position_world_space_, rotation_axis);
      const auto previous_intersection = ComputeRayPlaneIntersection(
          mouse_move_event->viewport()->CalculateViewRay(previous_mouse_position_screen_space), plane);
      const auto current_intersection = ComputeRayPlaneIntersection(
          mouse_move_event->viewport()->CalculateViewRay(current_mouse_position_screen_space), plane);

      if (!previous_intersection || !current_intersection) {
        // I don't know what to do here
        return;
      }

      const Vector3 previous_intersection_direction = Normalize(*previous_intersection - object_position_world_space_);
      const Vector3 current_intersection_direction = Normalize(*current_intersection - object_position_world_space_);
      const float absolute_angle = std::acos(Dot(previous_intersection_direction, current_intersection_direction));
      if (!std::isnan(absolute_angle)) {
        const float sign = Dot(Cross(previous_intersection_direction, current_intersection_direction), rotation_axis);
        const float angle = std::copysign(absolute_angle, sign);
        transform->Rotate(Normalize(rotation_axis), angle);
        const auto q = Quaternion::FromAxisAndAngle(Normalize(rotation_axis), angle);
      }

      float yaw, pitch, roll;
      transform->GetYawPitchRoll(&yaw, &pitch, &roll);
      current_tooltip_ = fmt::format("Yaw {}°, pitch {}°, roll {}°", yaw * RadiansToDegreesFactor<float>(),
                                     pitch * RadiansToDegreesFactor<float>(), roll * RadiansToDegreesFactor<float>());
      break;
    }

    case TransformationType::SCALE:
      Vector3 scale_direction_world_space;
      if (selected_axes_ == AxesSelection::X) {
        scale_direction_world_space = x_axis_world_space_;
      } else if (selected_axes_ == AxesSelection::Y) {
        scale_direction_world_space = y_axis_world_space_;
      } else if (selected_axes_ == AxesSelection::Z) {
        scale_direction_world_space = z_axis_world_space_;
      } else if (selected_axes_ == AxesSelection::XYZ) {
        scale_direction_world_space = x_axis_world_space_ + y_axis_world_space_ + z_axis_world_space_;
      }
      const Vector3 screen_offset = mouse_move_event->viewport()->WorldSpacePositionToScreenSpace(
          object_position_world_space_ + scale_direction_world_space);
      const Vector2 scale_direction_screen_space = Normalize(screen_offset - object_position_screen_space_);
      const float distance_in_scale_direction =
          Dot(scale_direction_screen_space, mouse_move_event->relative_screen_space_position());
      const float scale_factor = std::powf(1.01, distance_in_scale_direction);

      Vector3 scale = transform->scale();
      if (selected_axes_ == AxesSelection::X) {
        scale.x *= scale_factor;
      } else if (selected_axes_ == AxesSelection::Y) {
        scale.y *= scale_factor;
      } else if (selected_axes_ == AxesSelection::Z) {
        scale.z *= scale_factor;
      } else if (selected_axes_ == AxesSelection::XYZ) {
        scale *= scale_factor;
      }
      transform->SetScale(scale);

      current_tooltip_ = fmt::format("{}", transform->scale());
      break;
  }
}

}  // namespace editor
}  // namespace ovis
