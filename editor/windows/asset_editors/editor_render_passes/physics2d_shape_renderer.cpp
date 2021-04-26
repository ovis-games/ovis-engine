#include "../editing_controllers/object_selection_controller.hpp"
#include "selected_object_bounding_box.hpp"

#include <ovis/core/transform.hpp>
#include <ovis/rendering/rendering_viewport.hpp>
#include <ovis/physics2d/rigid_body2d.hpp>

namespace ovis {
namespace editor {

Physics2DShapeRenderer::Physics2DShapeRenderer(Scene* editing_scene)
    : PrimitiveRenderer(Name()), editing_scene_(editing_scene) {}

void Physics2DShapeRenderer::Render(const RenderContext& render_context) {
  if (viewport()->scene()->is_playing()) {
    return;
  }

  auto* object_selection_controller = editing_scene_->GetController<ObjectSelectionController>();
  SceneObject* object = object_selection_controller->selected_object();
  if (object == nullptr) {
    return;
  }

  RigidBody2D* rigid_body = object->GetComponent<RigidBody2D>("RigidBody2D");
  if (!rigid_body) {
    return;
  }

  if (rigid_body) {
    
  }

  BeginDraw(render_context);

  Transform* transform = object->GetComponent<Transform>("Transform");
  const Matrix3x4 local_to_world = transform ? transform->local_to_world_matrix() : Matrix3x4::IdentityTransformation();

  DrawLine(aabb_vertices[4], aabb_vertices[5], Color::White());
  DrawLine(aabb_vertices[6], aabb_vertices[7], Color::White());
  DrawLine(aabb_vertices[4], aabb_vertices[6], Color::White());
  DrawLine(aabb_vertices[5], aabb_vertices[7], Color::White());
  DrawLine(aabb_vertices[0], aabb_vertices[1], Color::White());
  DrawLine(aabb_vertices[2], aabb_vertices[3], Color::White());
  DrawLine(aabb_vertices[0], aabb_vertices[2], Color::White());
  DrawLine(aabb_vertices[1], aabb_vertices[3], Color::White());

  EndDraw();
}

}  // namespace editor
}  // namespace ovis
