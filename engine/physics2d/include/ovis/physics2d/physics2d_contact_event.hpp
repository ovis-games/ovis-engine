#pragma once

#include <box2d/b2_contact.h>

#include <ovis/core/event.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/physics2d/rigid_body2d.hpp>

namespace ovis {

class Physics2DContactEvent : public Event {
 public:
  inline Physics2DContactEvent(std::string_view type, b2Contact* contact) : Event(type), contact_(contact) {}

  inline bool is_touching() const { return contact_->IsTouching(); }

  inline bool is_enabled() const { return contact_->IsEnabled(); }
  inline void SetEnabled(bool enabled) { contact_->SetEnabled(enabled); }

  inline SceneObject* first_object() const {
    return reinterpret_cast<RigidBody2D*>(contact_->GetFixtureA()->GetUserData().pointer)->scene_object();
  }
  inline SceneObject* second_object() const {
    return reinterpret_cast<RigidBody2D*>(contact_->GetFixtureB()->GetUserData().pointer)->scene_object();
  }

  inline float friction() const { return contact_->GetFriction(); }
  inline void SetFriction(float friction) { contact_->SetFriction(friction); }
  inline void ResetFriction() { contact_->ResetFriction(); }

  inline float restitution() const { return contact_->GetRestitution(); }
  inline void SetRestitution(float restitution) { contact_->SetRestitution(restitution); }
  inline void ResetRestitution() { contact_->ResetRestitution(); }

  inline float restitution_threshold() const { return contact_->GetRestitutionThreshold(); }
  inline void SetRestitutionThreshold(float restitution_threshold) {
    contact_->SetRestitutionThreshold(restitution_threshold);
  }
  inline void ResetRestitutionThreshold() { contact_->ResetRestitutionThreshold(); }

  inline float tangent_speed() const { return contact_->GetTangentSpeed(); }
  inline void SetTangentSpeed(float tangent_speed) { contact_->SetTangentSpeed(tangent_speed); }

  static void RegisterType(sol::table* module);

 private:
  b2Contact* contact_;
};

}  // namespace ovis
