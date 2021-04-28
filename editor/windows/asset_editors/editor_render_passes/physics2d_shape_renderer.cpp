#include "physics2d_shape_renderer.hpp"

#include "../editing_controllers/object_selection_controller.hpp"
#include "../editing_controllers/physics2d_shape_controller.hpp"

#include <box2d/b2_chain_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_edge_shape.h>
#include <box2d/b2_polygon_shape.h>

#include <ovis/core/transform.hpp>
#include <ovis/rendering/rendering_viewport.hpp>
#include <ovis/physics2d/rigid_body2d.hpp>

namespace ovis {
namespace editor {

Physics2DShapeRenderer::Physics2DShapeRenderer(Scene* editing_scene)
    : PrimitiveRenderer(Name()), editing_scene_(editing_scene) {
  enable_alpha_blending_ = true;
}

void Physics2DShapeRenderer::Render(const RenderContext& render_context) {
  if (viewport()->scene()->is_playing()) {
    return;
  }

  auto* object_selection_controller = editing_scene_->GetController<ObjectSelectionController>();
  SceneObject* object = object_selection_controller->selected_object();
  if (object == nullptr) {
    return;
  }

  const RigidBody2DFixture* rigid_body_fixture = object->GetComponent<RigidBody2DFixture>("RigidBody2DFixture");
  if (!rigid_body_fixture) {
    return;
  }

  const b2Shape* shape = rigid_body_fixture->shape();
  BeginDraw(render_context);

  Transform* transform = object->GetComponent<Transform>("Transform");
  const Matrix3x4 local_to_world = transform ? transform->local_to_world_matrix() : Matrix3x4::IdentityTransformation();
  const Vector3 origin = TransformPosition(local_to_world, Vector3::Zero());
  const Color color_opaque = Color::Aqua();
  const Color color_transparent = Color(color_opaque.r, color_opaque.g, color_opaque.b, 0.5f);
  const Color color_selected = Color::Yellow();

  auto* controller = editing_scene_->GetController<Physics2DShapeController>();
  SDL_assert(controller != nullptr);

  switch (controller->type_) {
    case b2Shape::e_circle: {
      DrawDisc(origin, controller->radius_, color_transparent);
      break;
    }

    case b2Shape::e_polygon: {
      DrawConvexPolygon(controller->vertices_.data(), controller->vertices_.size(), color_transparent);
      for (const auto& vertex : IndexRange(controller->vertices_)) {
        const Color color = controller->selection_ == vertex.index() ? color_selected : color_opaque;
        DrawPoint(vertex.value(), 10.0f, color);
      }
      break;
    }

    case b2Shape::e_edge: {
      SDL_assert(controller->vertices_.size() == 4);
      DrawLine(controller->vertices_[1], controller->vertices_[2], color_transparent);

      // if (edge_shape->m_oneSided) {
      //   DrawLine(controller->vertices_[1], controller->vertices_[0], color_transparent);
      //   DrawLine(controller->vertices_[3], controller->vertices_[2], color_transparent);

      //   DrawPoint(controller->vertices_[0], 10.0f, color_opaque);
      //   DrawPoint(controller->vertices_[3], 10.0f, color_opaque);
      // }

      DrawPoint(controller->vertices_[1], 10.0f, controller->selection_ == 1 ? color_selected : color_opaque);
      DrawPoint(controller->vertices_[2], 10.0f, controller->selection_ == 2 ? color_selected : color_opaque);
      break;
    }

    case b2Shape::e_chain: {
      DrawLineStip(controller->vertices_, color_transparent);
      for (const auto& vertex : IndexRange(controller->vertices_)) {
        const Color color = controller->selection_ == vertex.index() ? color_selected : color_opaque;
        DrawPoint(vertex.value(), 10.0f, color);
      }
      break;
    }

    case b2Shape::e_typeCount:
      SDL_assert(false);
      break;
  }

  EndDraw();
}

}  // namespace editor
}  // namespace ovis
