#pragma once

#include <vector>
#include <variant>

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_shape.h>

#include <ovis/utils/range.hpp>
#include <ovis/core/scene_object_component.hpp>
#include <ovis/physics2d/box2d.hpp>
#include <ovis/physics2d/rigid_body2d_fixture.hpp>

namespace ovis {

class RigidBody2D : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();
  friend class PhysicsWorld2D;
  friend class RigidBody2DFixture;

 public:
  RigidBody2D(SceneObject* object);
  ~RigidBody2D();

  inline void ApplyForce(Vector2 force) { EnforceValidBody()->ApplyForceToCenter(ToBox2DVec2(force), true); }
  inline void ApplyForce(Vector2 force, Vector2 point) {
    EnforceValidBody()->ApplyForce(ToBox2DVec2(force), ToBox2DVec2(point), true);
  }
  inline void ApplyTorque(float torque) { EnforceValidBody()->ApplyTorque(torque, true); }
  inline void ApplyLinearImpulse(Vector2 impulse) {
    EnforceValidBody()->ApplyLinearImpulseToCenter(ToBox2DVec2(impulse), true);
  }
  inline void ApplyLinearImpulse(Vector2 impulse, Vector2 point) {
    EnforceValidBody()->ApplyLinearImpulse(ToBox2DVec2(impulse), ToBox2DVec2(point), true);
  }
  inline void ApplyAngularImpulse(float impulse) { EnforceValidBody()->ApplyAngularImpulse(impulse, true); }

  inline float mass() const { return EnforceValidBody()->GetMass(); }
  inline float inertia() const { return EnforceValidBody()->GetInertia(); }
  inline Vector2 center_of_mass() const {
    b2MassData data;
    EnforceValidBody()->GetMassData(&data);
    return FromBox2DVec2(data.center);
  }

  inline void SetLinearVelocity(Vector2 v) {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      std::get<b2Body*>(body_)->SetLinearVelocity(ToBox2DVec2(v));
    } else {
      std::get<0>(body_)->linearVelocity = ToBox2DVec2(v);
    }
  }
  inline Vector2 linear_velocity() const {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      return FromBox2DVec2(std::get<1>(body_)->GetLinearVelocity());
    } else {
      return FromBox2DVec2(std::get<0>(body_)->linearVelocity);
    }
  }

  inline void SetAngularVelocity(float omega) {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      std::get<b2Body*>(body_)->SetAngularVelocity(omega);
    } else {
      std::get<0>(body_)->angularVelocity = omega;
    }
  }
  inline float angular_velocity() const {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      return std::get<1>(body_)->GetAngularVelocity();
    } else {
      return std::get<0>(body_)->angularVelocity;
    }
  }

  inline void SetLinearDamping(float damping) {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      std::get<b2Body*>(body_)->SetLinearDamping(damping);
    } else {
      std::get<0>(body_)->linearDamping = damping;
    }
  }
  inline float linear_damping() const {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      return std::get<1>(body_)->GetLinearDamping();
    } else {
      return std::get<0>(body_)->linearDamping;
    }
  }

  inline void SetAngularDamping(float damping) {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      std::get<b2Body*>(body_)->SetAngularDamping(damping);
    } else {
      std::get<0>(body_)->angularDamping = damping;
    }
  }
  inline float angular_damping() const {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      return std::get<1>(body_)->GetAngularDamping();
    } else {
      return std::get<0>(body_)->angularDamping;
    }
  }

  inline void SetGravityScale(float scale) {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      std::get<b2Body*>(body_)->SetGravityScale(scale);
    } else {
      std::get<0>(body_)->gravityScale = scale;
    }
  }
  inline float gravity_scale() const {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      return std::get<1>(body_)->GetGravityScale();
    } else {
      return std::get<0>(body_)->gravityScale;
    }
  }

  inline void SetType(b2BodyType type) {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      std::get<b2Body*>(body_)->SetType(type);
    } else {
      std::get<0>(body_)->type = type;
    }
  }
  inline b2BodyType type() const {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      return std::get<1>(body_)->GetType();
    } else {
      return std::get<0>(body_)->type;
    }
  }

  inline void SetIsBullet(bool is_bullet) {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      std::get<b2Body*>(body_)->SetBullet(is_bullet);
    } else {
      std::get<0>(body_)->bullet = is_bullet;
    }
  }
  inline bool is_bullet() const {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      return std::get<1>(body_)->IsBullet();
    } else {
      return std::get<0>(body_)->bullet;
    }
  }

  inline void SetAllowSleep(bool allow_sleep) {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      std::get<b2Body*>(body_)->SetSleepingAllowed(allow_sleep);
    } else {
      std::get<0>(body_)->allowSleep = allow_sleep;
    }
  }
  inline bool allow_sleep() const {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      return std::get<1>(body_)->IsSleepingAllowed();
    } else {
      return std::get<0>(body_)->allowSleep;
    }
  }

  inline void SetIsAwake(bool is_awake) {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      std::get<b2Body*>(body_)->SetAwake(is_awake);
    } else {
      std::get<0>(body_)->awake = is_awake;
    }
  }
  inline bool is_awake() const {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      return std::get<1>(body_)->IsAwake();
    } else {
      return std::get<0>(body_)->awake;
    }
  }

  inline void SetIsEnabled(bool is_enabled) {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      std::get<b2Body*>(body_)->SetEnabled(is_enabled);
    } else {
      std::get<0>(body_)->enabled = is_enabled;
    }
  }
  inline bool is_enabled() const {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      return std::get<1>(body_)->IsEnabled();
    } else {
      return std::get<0>(body_)->enabled;
    }
  }

  inline void SetIsRotationFixed(bool is_rotation_fixed) {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      std::get<b2Body*>(body_)->SetFixedRotation(is_rotation_fixed);
    } else {
      std::get<0>(body_)->fixedRotation = is_rotation_fixed;
    }
  }
  inline bool is_rotation_fixed() const {
    if (std::holds_alternative<b2Body*>(body_)) [[likely]] {
      return std::get<1>(body_)->IsFixedRotation();
    } else {
      return std::get<0>(body_)->fixedRotation;
    }
  }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &SCHEMA; }

  static void RegisterType(sol::table* module);

 private:
  std::variant<std::unique_ptr<b2BodyDef>, b2Body*> body_;

  static const json SCHEMA;

  inline b2Body* EnforceValidBody() const {
    if (!std::holds_alternative<b2Body*>(body_)) {
      throw std::runtime_error("Body internals are not created yet. When the component was created within a physics2D callback you need to wait until after the physics update to call this function.");
    }
    return std::get<b2Body*>(body_);
  }

  static inline std::string TypeToString(b2BodyType type) {
    switch (type) {
      case b2BodyType::b2_staticBody:
        return "static";
      case b2BodyType::b2_kinematicBody:
        return "kinematic";
      case b2BodyType::b2_dynamicBody:
        return "dynamic";
    }
  }

  static inline b2BodyType StringToType(std::string_view string) {
    if (string == "static") {
      return b2BodyType::b2_staticBody;
    } else if (string == "kinematic") {
      return b2BodyType::b2_kinematicBody;
    } else if (string == "dynamic") {
      return b2BodyType::b2_dynamicBody;
    } else {
      throw std::runtime_error("Invalid type!");
      return b2BodyType::b2_staticBody;
    }
  }

  void CreateInternals(const b2BodyDef* definition);
};

}  // namespace ovis
