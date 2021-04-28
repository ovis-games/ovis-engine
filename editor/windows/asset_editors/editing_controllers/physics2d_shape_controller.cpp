#include "physics2d_shape_controller.hpp"

#include "object_selection_controller.hpp"

#include <box2d/b2_chain_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_edge_shape.h>
#include <box2d/b2_polygon_shape.h>

#include <ovis/utils/log.hpp>
#include <ovis/input/mouse_events.hpp>

namespace ovis {
namespace editor {

Physics2DShapeController::Physics2DShapeController() : EditorController(Name()) {
  SubscribeToEvent(MouseButtonPressEvent::TYPE);
  SubscribeToEvent(MouseButtonReleaseEvent::TYPE);
  SubscribeToEvent(MouseMoveEvent::TYPE);
}

void Physics2DShapeController::Update(std::chrono::microseconds) {
  SceneObject* object = GetSelectedObject(scene());
  if (object == nullptr) {
    fixture_.reset();
    return;
  }

  fixture_ = object->GetComponent<RigidBody2DFixture>("RigidBody2DFixture");
  if (fixture_ == nullptr) {
    return;
  }

  Transform* transform = object->GetComponent<Transform>("Transform");
  const Matrix3x4 local_to_world = transform ? transform->local_to_world_matrix() : Matrix3x4::IdentityTransformation();

  auto shape = fixture_->shape();
  type_ = shape->GetType();
  radius_ = 0.0f;
  vertices_.clear();

  switch (type_) {
    case b2Shape::e_circle: {
      auto circle_shape = static_cast<const b2CircleShape*>(shape);
      radius_ = circle_shape->m_radius;
      break;
    }

    case b2Shape::e_polygon: {
      auto polygon_shape = static_cast<const b2PolygonShape*>(shape);
      vertices_.reserve(polygon_shape->m_count);
      std::transform(polygon_shape->m_vertices, polygon_shape->m_vertices + polygon_shape->m_count,
                     std::back_inserter(vertices_), [&](const auto& point) {
                       return TransformPosition(local_to_world, Vector3::FromVector2(FromBox2DVec2(point)));
                     });
      break;
    }

    case b2Shape::e_edge: {
      auto edge_shape = static_cast<const b2EdgeShape*>(shape);

      vertices_ = {TransformPosition(local_to_world, Vector3::FromVector2(FromBox2DVec2(edge_shape->m_vertex0))),
                   TransformPosition(local_to_world, Vector3::FromVector2(FromBox2DVec2(edge_shape->m_vertex1))),
                   TransformPosition(local_to_world, Vector3::FromVector2(FromBox2DVec2(edge_shape->m_vertex2))),
                   TransformPosition(local_to_world, Vector3::FromVector2(FromBox2DVec2(edge_shape->m_vertex3)))};
      break;
    }

    case b2Shape::e_chain: {
      auto chain_shape = static_cast<const b2ChainShape*>(shape);
      vertices_.reserve(chain_shape->m_count);
      std::transform(chain_shape->m_vertices, chain_shape->m_vertices + chain_shape->m_count,
                     std::back_inserter(vertices_), [&](const auto& point) {
                       return TransformPosition(local_to_world, Vector3::FromVector2(FromBox2DVec2(point)));
                     });
      break;
    }

    case b2Shape::e_typeCount:
      SDL_assert(false);
      break;
  }
}

void Physics2DShapeController::ProcessEvent(Event* event) {
  if (event->type() == MouseMoveEvent::TYPE) {
    auto mouse_move_event = static_cast<MouseMoveEvent*>(event);
    auto screen_space_mouse_position = mouse_move_event->screen_space_position();
    const Vector3 mouse_world_space_position = mouse_move_event->viewport()->ScreenSpacePositionToWorldSpace(
        Vector3::FromVector2(screen_space_mouse_position));

    if (!is_dragging_) {
      selection_.reset();
      for (const auto& vertex : IndexRange(vertices_)) {
        const Vector2 screen_space_vertex =
            mouse_move_event->viewport()->WorldSpacePositionToScreenSpace(vertex.value());
        if (SquaredDistance(screen_space_vertex, screen_space_mouse_position) < 10 * 10) {
          selection_ = vertex.index();
          mouse_move_event->StopPropagation();
        }
      }
    } else {
      // TODO: move stuff
    }
  } else if (event->type() == MouseButtonPressEvent::TYPE) {
    auto mouse_button_event = static_cast<MouseButtonPressEvent*>(event);
    if (mouse_button_event->button() == MouseButton::Left() && selection_.has_value()) {
      is_dragging_ = true;
      mouse_button_event->StopPropagation();
    }
  } else if (event->type() == MouseButtonReleaseEvent::TYPE) {
    auto mouse_button_event = static_cast<MouseButtonReleaseEvent*>(event);
    if (mouse_button_event->button() == MouseButton::Left() && is_dragging_) {
      is_dragging_ = false;
      mouse_button_event->StopPropagation();
    }
  }
}

}  // namespace editor
}  // namespace ovis
