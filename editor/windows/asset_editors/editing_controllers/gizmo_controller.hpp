#pragma once

#include "editor_controller.hpp"

#include <ovis/core/scene.hpp>
#include <ovis/input/mouse_events.hpp>

namespace ovis {
namespace editor {

struct LineSegment2D {
  Vector2 endpoints[2];
};

float DistanceToLineSegment(Vector2 point, const LineSegment2D& line_segment);

class GizmoController : public EditorController {
  friend class GizmoRenderer;

 public:
  static constexpr std::string_view Name() { return "GizmoController"; }

  enum class GizmoType { MOVE, ROTATE, SCALE };
  enum class AxisSelection {
    NOTHING,
    X,
    Y,
    Z,
    XYZ,
  };
  enum class CoordinateSystem { OBJECT, WORLD };

  GizmoController();

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

  inline void SetGizmoType(GizmoType type) { type_ = type; }
  inline GizmoType gizmo_type() { return type_; }

  inline void SetCoordinateSystem(CoordinateSystem coordinate_system) { coordinate_system_ = coordinate_system; }
  inline void SwitchCoordinateSystem() {
    if (coordinate_system_ == CoordinateSystem::WORLD) {
      coordinate_system_ = CoordinateSystem::OBJECT;
    } else {
      coordinate_system_ = CoordinateSystem::WORLD;
    }
  }
  inline CoordinateSystem coordinate_system() { return coordinate_system_; }

 private:
  GizmoType type_ = GizmoType::MOVE;
  CoordinateSystem coordinate_system_ = CoordinateSystem::OBJECT;

  std::string current_tooltip_;

  // TODO: safe reference to object
  bool object_selected_ = false;
  AxisSelection selected_axes_;

  Vector3 object_position_screen_space_;
  Vector3 x_axis_endpoint_screen_space_;
  Vector3 y_axis_endpoint_screen_space_;
  Vector3 z_axis_endpoint_screen_space_;

  Vector3 object_position_world_space_;
  Vector3 x_axis_world_space_;
  Vector3 y_axis_world_space_;
  Vector3 z_axis_world_space_;

  float gizmo_radius_screen_space_ = 75.0f;
  float line_thickness_screen_space_ = 5.0f;
  float point_size_screen_space_ = 9.0f;

  // how many screen space pixels is covered by 1 unit in world space
  float world_to_screen_space_factor_;

  bool CheckMousePosition(Vector2 position);
  void HandleDragging(MouseMoveEvent* mouse_move_event);
};

}  // namespace editor
}  // namespace ovis
