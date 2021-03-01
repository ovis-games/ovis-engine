#pragma once

#include <box2d/b2_world.h>

#include <ovis/engine/scene_controller.hpp>

namespace ovis {

class PhysicsWorld2D : public SceneController {
 public:
  PhysicsWorld2D();

  void Update(std::chrono::microseconds delta_time) override;
  bool ProcessEvent(const SDL_Event& event) override;

 private:
  b2World world_;
};

}  // namespace ovis
