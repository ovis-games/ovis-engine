#include <ovis/physics2d/physics2d_begin_contact_event.hpp>

namespace ovis {

void Physics2DBeginContactEvent::RegisterType(sol::table* module) {
  /// An event that occurs when two objects get in contact.
  // @classmod ovis.physics2d.BeginContactEvent
  sol::usertype<Physics2DBeginContactEvent> begin_contact_event_type = module->new_usertype<Physics2DBeginContactEvent>(
      "BeginContactEvent", sol::no_constructor, sol::base_classes, sol::bases<Event, Physics2DContactEvent>());
}

}  // namespace ovis
