#include "physics2d_shape_controller.hpp"

#include "object_selection_controller.hpp"
#include "transformation_tools_controller.hpp"

#include <box2d/b2_chain_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_edge_shape.h>
#include <box2d/b2_polygon_shape.h>

#include <ovis/utils/log.hpp>
#include <ovis/core/math.hpp>
#include <ovis/input/mouse_events.hpp>

namespace ovis {
namespace editor {

Physics2DShapeController::Physics2DShapeController() : EditorController(Name()) {
  SubscribeToEvent(MouseButtonPressEvent::TYPE);
  SubscribeToEvent(MouseButtonReleaseEvent::TYPE);
  SubscribeToEvent(MouseMoveEvent::TYPE);
  UpdateBefore(TransformationToolsController::Name());
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

  const Transform* transform = object->GetComponent<Transform>("Transform");
  body_to_world_ = transform ? transform->local_to_world_matrix() : Matrix3x4::IdentityTransformation();
  world_to_body_ = InvertAffine(body_to_world_);

  if (!is_dragging_) {
    const b2Shape* shape = fixture_->shape();
    type_ = shape->GetType();
    radius_ = 0.0f;
    vertices_.clear();

    switch (type_) {
      case b2Shape::e_circle: {
        auto circle_shape = static_cast<const b2CircleShape*>(shape);
        radius_ = circle_shape->m_radius;
        vertices_ = {FromBox2DVec2(circle_shape->m_p)};
        break;
      }

      case b2Shape::e_polygon: {
        auto polygon_shape = static_cast<const b2PolygonShape*>(shape);
        vertices_.reserve(polygon_shape->m_count);
        std::transform(polygon_shape->m_vertices, polygon_shape->m_vertices + polygon_shape->m_count,
                       std::back_inserter(vertices_), FromBox2DVec2);
        break;
      }

      case b2Shape::e_edge: {
        auto edge_shape = static_cast<const b2EdgeShape*>(shape);

        one_sided_edge_ = edge_shape->m_oneSided;
        vertices_ = {FromBox2DVec2(edge_shape->m_vertex0), FromBox2DVec2(edge_shape->m_vertex1),
                     FromBox2DVec2(edge_shape->m_vertex2), FromBox2DVec2(edge_shape->m_vertex3)};
        break;
      }

      case b2Shape::e_chain: {
        auto chain_shape = static_cast<const b2ChainShape*>(shape);
        vertices_.reserve(chain_shape->m_count + 2);
        vertices_.push_back(FromBox2DVec2(chain_shape->m_prevVertex));
        std::transform(chain_shape->m_vertices, chain_shape->m_vertices + chain_shape->m_count,
                       std::back_inserter(vertices_), FromBox2DVec2);
        vertices_.push_back(FromBox2DVec2(chain_shape->m_nextVertex));
        break;
      }

      case b2Shape::e_typeCount:
        SDL_assert(false);
        break;
    }
  }
}

void Physics2DShapeController::ProcessEvent(Event* event) {
  if (fixture_ == nullptr) {
    return;
  }

  if (event->type() == MouseMoveEvent::TYPE) {
    auto mouse_move_event = static_cast<MouseMoveEvent*>(event);
    auto screen_space_mouse_position = mouse_move_event->screen_space_position();
    const Vector2 mouse_world_space_position = mouse_move_event->viewport()->ScreenSpacePositionToWorldSpace(
        Vector3::FromVector2(screen_space_mouse_position));

    if (!is_dragging_) {
      selection_.reset();
      if (type_ == b2Shape::e_circle) {
        SDL_assert(vertices_.size() == 1);
        const float squared_distance_to_midpoint = SquaredDistance<Vector2>(
            mouse_world_space_position, TransformPosition(body_to_world_, Vector3::FromVector2(vertices_[0])));
        const float squared_radius = radius_ * radius_;
        if (std::abs(squared_distance_to_midpoint - squared_radius) < 10.0f) {
          selection_ = 0;
        }
      } else {
        for (const auto& vertex : IndexRange(vertices_)) {
          const Vector2 screen_space_vertex = mouse_move_event->viewport()->WorldSpacePositionToScreenSpace(
              TransformPosition(body_to_world_, Vector3::FromVector2(vertex.value())));
          if (SquaredDistance(screen_space_vertex, screen_space_mouse_position) < 10 * 10) {
            selection_ = vertex.index();
            mouse_move_event->StopPropagation();
          }
        }
      }
    } else if (type_ != fixture_->shape()->GetType()) {
      is_dragging_ = false;
    } else {
      error_vertex_.reset();

      if (type_ == b2Shape::e_circle) {
        SDL_assert(vertices_.size() == 1);
        radius_ = Distance<Vector2>(mouse_world_space_position,
                                    TransformPosition(body_to_world_, Vector3::FromVector2(vertices_[0])));
        fixture_->shape()->m_radius = radius_;
      } else {
        SDL_assert(selection_.has_value());
        SDL_assert(*selection_ < vertices_.size());

        const Vector2 old_position = vertices_[*selection_];
        vertices_[*selection_] = TransformPosition(world_to_body_, Vector3::FromVector2(mouse_world_space_position));
        std::vector<b2Vec2> points;
        points.reserve(vertices_.size());
        std::transform(vertices_.begin(), vertices_.end(), std::back_inserter(points),
                       [&](const auto& vertex) { return ToBox2DVec2(vertex); });
        if (type_ == b2Shape::e_polygon) {
          if (IsConvex(vertices_)) {
            static_cast<b2PolygonShape*>(fixture_->shape())->Set(points.data(), points.size());
          } else {
            error_vertex_ = vertices_[*selection_];
            vertices_[*selection_] = old_position;
          }
        } else if (type_ == b2Shape::e_edge) {
          SDL_assert(vertices_.size() == 4);
          auto edge_shape = static_cast<b2EdgeShape*>(fixture_->shape());
          edge_shape->m_vertex0 = points[0];
          edge_shape->m_vertex1 = points[1];
          edge_shape->m_vertex2 = points[2];
          edge_shape->m_vertex3 = points[3];
        } else if (type_ == b2Shape::e_chain) {
          auto chain_shape = static_cast<b2ChainShape*>(fixture_->shape());
          SDL_assert(points.size() == chain_shape->m_count + 2);
          chain_shape->m_prevVertex = points.front();
          for (int i = 0; i < chain_shape->m_count; ++i) {
            chain_shape->m_vertices[i] = points[i + 1];
          }
          chain_shape->m_nextVertex = points.back();
        }
      }
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
      error_vertex_.reset();
      is_dragging_ = false;
      SubmitChangesToScene();
      mouse_button_event->StopPropagation();
    }
  }
}

}  // namespace editor
}  // namespace ovis
