#pragma once

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_shape.h>

#include <ovis/core/scene_object_component.hpp>
#include <ovis/physics2d/box2d.hpp>

namespace ovis {

class RigidBody2D : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();
  friend class PhysicsWorld2D;

 public:
  RigidBody2D();
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
    if (body_ == nullptr) {
      body_definition_.linearVelocity = ToBox2DVec2(v);
    } else {
      body_->SetLinearVelocity(ToBox2DVec2(v));
    }
  }
  inline Vector2 linear_velocity() const {
    if (body_ == nullptr) {
      return FromBox2DVec2(body_definition_.linearVelocity);
    } else {
      return FromBox2DVec2(body_->GetLinearVelocity());
    }
  }

  inline void SetAngularVelocity(float omega) {
    if (body_ == nullptr) {
      body_definition_.angularVelocity = omega;
    } else {
      body_->SetAngularVelocity(omega);
    }
  }
  inline float angular_velocity() const {
    if (body_ == nullptr) {
      return body_definition_.angularVelocity;
    } else {
      return body_->GetAngularVelocity();
    }
  }

  inline void SetLinearDamping(float damping) {
    if (body_ == nullptr) {
      body_definition_.linearDamping = damping;
    } else {
      body_->SetLinearDamping(damping);
    }
  }
  inline float linear_damping() const {
    if (body_ == nullptr) {
      return body_definition_.linearDamping;
    } else {
      return body_->GetLinearDamping();
    }
  }

  inline void SetAngularDamping(float damping) {
    if (body_ == nullptr) {
      body_definition_.angularDamping = damping;
    } else {
      body_->SetAngularDamping(damping);
    }
  }
  inline float angular_damping() const {
    if (body_ == nullptr) {
      return body_definition_.angularDamping;
    } else {
      return body_->GetAngularDamping();
    }
  }

  inline void SetGravityScale(float scale) {
    if (body_ == nullptr) {
      body_definition_.gravityScale = scale;
    } else {
      body_->SetGravityScale(scale);
    }
  }
  inline float gravity_scale() const {
    if (body_ == nullptr) {
      return body_definition_.gravityScale;
    } else {
      return body_->GetGravityScale();
    }
  }

  inline void SetType(b2BodyType type) {
    if (body_ == nullptr) {
      body_definition_.type = type;
    } else {
      body_->SetType(type);
    }
  }
  inline b2BodyType type() const {
    if (body_ == nullptr) {
      return body_definition_.type;
    } else {
      return body_->GetType();
    }
  }

  inline void SetIsBullet(bool is_bullet) {
    if (body_ == nullptr) {
      body_definition_.bullet = is_bullet;
    } else {
      body_->SetBullet(is_bullet);
    }
  }
  inline bool is_bullet() const {
    if (body_ == nullptr) {
      return body_definition_.bullet;
    } else {
      return body_->IsBullet();
    }
  }

  inline void SetAllowSleep(bool allow_sleep) {
    if (body_ == nullptr) {
      body_definition_.allowSleep = allow_sleep;
    } else {
      body_->SetSleepingAllowed(allow_sleep);
    }
  }
  inline bool allow_sleep() const {
    if (body_ == nullptr) {
      return body_definition_.allowSleep;
    } else {
      return body_->IsSleepingAllowed();
    }
  }

  inline void SetIsAwake(bool is_awake) {
    if (body_ == nullptr) {
      body_definition_.awake = is_awake;
    } else {
      body_->SetBullet(is_awake);
    }
  }
  inline bool is_awake() const {
    if (body_ == nullptr) {
      return body_definition_.awake;
    } else {
      return body_->IsAwake();
    }
  }

  inline void SetIsEnabled(bool is_enabled) {
    if (body_ == nullptr) {
      body_definition_.enabled = is_enabled;
    } else {
      body_->SetBullet(is_enabled);
    }
  }
  inline bool is_enabled() const {
    if (body_ == nullptr) {
      return body_definition_.enabled;
    } else {
      return body_->IsEnabled();
    }
  }

  inline void SetIsRotationFixed(bool is_rotation_fixed) {
    if (body_ == nullptr) {
      body_definition_.fixedRotation = is_rotation_fixed;
    } else {
      body_->SetBullet(is_rotation_fixed);
    }
  }
  inline bool is_rotation_fixed() const {
    if (body_ == nullptr) {
      return body_definition_.fixedRotation;
    } else {
      return body_->IsFixedRotation();
    }
  }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &SCHEMA; }

  static void RegisterType(sol::table* module);

 private:
  std::unique_ptr<b2Shape> shape_;
  b2BodyDef body_definition_;
  b2Body* body_ = nullptr;
  b2FixtureDef fixture_definition_;
  b2Fixture* fixture_ = nullptr;

  static const json SCHEMA;

  json SerializeShape() const;
  void DeserializeShape(const json& data);

  inline b2Body* EnforceValidBody() const {
    if (body_ == nullptr) {
      throw std::runtime_error("");
    }
    return body_;
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
};

}  // namespace ovis
