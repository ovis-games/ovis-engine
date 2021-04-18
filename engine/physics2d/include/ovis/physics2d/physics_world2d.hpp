#pragma once

#include <box2d/b2_world.h>

#include <ovis/core/scene_controller.hpp>

namespace ovis {

class PhysicsWorld2D : public SceneController {
  friend class Physics2DDebugLayer;

 public:
  PhysicsWorld2D();

  void Update(std::chrono::microseconds delta_time) override;

  const json* GetSchema() const override;
  json Serialize() const override;
  bool Deserialize(const json& data) override;

 private:
  b2World world_;

  static const json SCHEMA;
};

}  // namespace ovis
