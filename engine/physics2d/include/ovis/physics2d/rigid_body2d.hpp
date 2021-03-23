#pragma once

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_shape.h>

#include <ovis/core/scene_object_component.hpp>

namespace ovis {

class RigidBody2D : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE(RigidBody2D);
  friend class PhysicsWorld2D;

 public:
  RigidBody2D();

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &SCHEMA; }

 private:
  b2BodyDef body_definition_;
  std::unique_ptr<b2Shape> shape_;
  b2Body* body_ = nullptr;
  b2FixtureDef fixture_definition_;
  b2Fixture* fixture_ = nullptr;

  static const json SCHEMA;

  json SerializeShape() const;
  void DeserializeShape(const json& data);
};

}  // namespace ovis
