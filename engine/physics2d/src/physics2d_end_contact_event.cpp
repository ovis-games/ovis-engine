#include <ovis/physics2d/physics2d_end_contact_event.hpp>

namespace ovis {

void Physics2DEndContactEvent::RegisterType(sol::table* module) {
  /// An event that occurs when two objects get in contact.
  // @classmod ovis.physics2d.EndContactEvent
  sol::usertype<Physics2DEndContactEvent> end_contact_event_type = module->new_usertype<Physics2DEndContactEvent>(
      "EndContactEvent", sol::no_constructor, sol::base_classes, sol::bases<Event, Physics2DContactEvent>());
}

}  // namespace ovis
