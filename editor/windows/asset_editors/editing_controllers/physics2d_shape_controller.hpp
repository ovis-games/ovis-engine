#pragma once

#include "editor_controller.hpp"

#include <ovis/utils/safe_pointer.hpp>
#include <ovis/core/intersection.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/scene_controller.hpp>
#include <ovis/physics2d/rigid_body2d_fixture.hpp>

namespace ovis {
namespace editor {

class Physics2DShapeController : public EditorController {
  friend class Physics2DShapeRenderer;

 public:
  static constexpr std::string_view Name() { return "Physics2DShapeController"; }

  Physics2DShapeController();

  void Update(std::chrono::microseconds) override;
  void ProcessEvent(Event* event) override;

 private:
  b2Shape::Type type_;
  safe_ptr<RigidBody2DFixture> fixture_;
  std::vector<Vector3> vertices_;
  float radius_ = 0.0f;
  std::optional<size_t> selection_;
  bool is_dragging_;
};

}  // namespace editor
}  // namespace ovis
