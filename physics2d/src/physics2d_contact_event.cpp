#include <ovis/physics2d/physics2d_contact_event.hpp>

namespace ovis {

void Physics2DContactEvent::RegisterType(sol::table* module) {
  /// An event that occurs when two objects get in contact.
  // @classmod ovis.physics2d.ContactEvent
  sol::usertype<Physics2DContactEvent> contact_event_type = module->new_usertype<Physics2DContactEvent>(
      "ContactEvent", sol::no_constructor, sol::base_classes, sol::bases<Event>());

  /// Is this contact touching?
  // @field[type=bool] is_touching
  contact_event_type["is_touching"] = sol::property(&Physics2DContactEvent::is_touching);

  /// Has this contact been disabled?
  // @field[type=bool] is_enabled
  contact_event_type["is_enabled"] =
      sol::property(&Physics2DContactEvent::is_enabled, &Physics2DContactEvent::SetEnabled);

  /// The first of the two objects that made contact.
  // @field[type=ovis.core.SceneObject] first_object
  contact_event_type["first_object"] = sol::property(&Physics2DContactEvent::first_object);

  /// The second of the two objects that made contact.
  // @field[type=ovis.core.SceneObject] second_object
  contact_event_type["second_object"] = sol::property(&Physics2DContactEvent::second_object);

  /// The friction between the objects.
  // @field[type=number] friction
  contact_event_type["friction"] = sol::property(&Physics2DContactEvent::friction, &Physics2DContactEvent::SetFriction);

  /// Reset the friction mixture to the default value.
  // @function reset_friction
  contact_event_type["reset_friction"] = &Physics2DContactEvent::ResetFriction;

  /// Restitution.
  // @field[type=number] restitution
  contact_event_type["restitution"] =
      sol::property(&Physics2DContactEvent::restitution, &Physics2DContactEvent::SetRestitution);

  /// Reset the restitution to the default value.
  // @function reset_restitution
  contact_event_type["reset_restitution"] = &Physics2DContactEvent::ResetRestitution;

  /// Restitution threshold.
  // @field[type=number] restitution_threshold
  contact_event_type["restitution_threshold"] =
      sol::property(&Physics2DContactEvent::restitution_threshold, &Physics2DContactEvent::SetRestitutionThreshold);

  /// Reset the restitution_threshold to the default value.
  // @function reset_restitution_threshold
  contact_event_type["reset_restitution_threshold"] = &Physics2DContactEvent::ResetRestitutionThreshold;

  /// The desired tangent speed for a conveyor belt behavior.
  // In meters per second.
  // @field[type=number] tangent_speed
  contact_event_type["tangent_speed"] =
      sol::property(&Physics2DContactEvent::tangent_speed, &Physics2DContactEvent::SetTangentSpeed);
}

}  // namespace ovis
