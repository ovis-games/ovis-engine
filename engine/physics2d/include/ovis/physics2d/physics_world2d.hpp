#pragma once

#include <box2d/b2_world.h>
#include <box2d/b2_world_callbacks.h>

#include <ovis/core/scene_controller.hpp>

namespace ovis {

class PhysicsWorld2D : public SceneController, public b2ContactListener {
  friend class Physics2DDebugLayer;

 public:
  PhysicsWorld2D();

  void Update(std::chrono::microseconds delta_time) override;

  const json *GetSchema() const override;
  json Serialize() const override;
  bool Deserialize(const json &data) override;

  // b2ContactListener methods
  void BeginContact(b2Contact* contact) override;
  void EndContact(b2Contact* contact) override;
  void PreSolve(b2Contact* contact, const b2Manifold* old_manifold) override;
  void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

 private:
  b2World world_;

  static const json SCHEMA;
};

}  // namespace ovis
