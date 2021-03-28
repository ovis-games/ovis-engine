#pragma once

#include <ovis/core/scene.hpp>
#include <ovis/core/scene_controller.hpp>

namespace ovis {
namespace editor {

struct LineSegment2D {
  Vector2 endpoints[2];
};

float DistanceToLineSegment(Vector2 point, const LineSegment2D& line_segment);

class GizmoController : public SceneController {
  friend class GizmoRenderer;

 public:
  static constexpr std::string_view Name() { return "GizmoController"; }

  enum class GizmoType { MOVE, ROTATE, SCALE };
  enum class MovementSelection {
    NOTHING,
    X,
    Y,
    Z,
    XYZ,
  };

  GizmoController(Scene* game_scene);

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

  inline void SetGizmoType(GizmoType type) { type_ = type; }
  inline GizmoType gizmo_type() { return type_; }

 private:
  Scene* game_scene_;
  GizmoType type_ = GizmoType::MOVE;

  // TODO: safe reference to object
  bool object_selected_ = false;
  MovementSelection movement_selection_;
  Vector3 object_position_;
  LineSegment2D line_x_;
  LineSegment2D line_y_;
  LineSegment2D line_z_;
  float gizmo_radius_ = 75.0f;
  float line_thickness_ = 5.0f;
  float point_size_ = 9.0f;

  bool CheckMousePosition(Vector2 position);
};

}  // namespace editor
}  // namespace ovis
