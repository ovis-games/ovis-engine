#pragma once

#include <string>

#include <ovis/rendering/primitive_renderer.hpp>

namespace ovis {
namespace editor {

class GizmoRenderer : public PrimitiveRenderer {
 public:
  GizmoRenderer(Scene* editing_scene);

  void Render(const RenderContext& render_context) override;

  inline void SetGizmoRadius(float gizmo_radius) { gizmo_radius_ = gizmo_radius; }
  inline float gizmo_radius() const { return gizmo_radius_; }

 private:
  Scene* editing_scene_;
  float gizmo_radius_ = 75.0f;
};

}  // namespace editor
}  // namespace ovis
