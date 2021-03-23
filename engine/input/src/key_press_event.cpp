#include <ovis/input/key_events.hpp>

namespace ovis {

void KeyPressEvent::RegisterType(sol::table* module) {
  /// Occurs when a key is pressed.
  // @classmod ovis.input.KeyPressEvent
  // @base ovis.core.Event
  // @usage local input = require "ovis.input"
  // local KeyPressEvent = input.KeyPressEvent
  sol::usertype<KeyPressEvent> key_event_type =
      module->new_usertype<KeyPressEvent>("KeyPressEvent", sol::no_constructor, sol::base_classes, sol::bases<Event>());

  /// The key that is pressed.
  // @field[type=Key] key
  key_event_type["key"] = sol::property(&KeyPressEvent::key);
}

}  // namespace ovis
