#include <ovis/input/key_events.hpp>

namespace ovis {

void KeyReleaseEvent::RegisterType(sol::table* module) {
  /// Occurs when a key is released.
  // @classmod ovis.input.KeyReleaseEvent
  // @base ovis.core.Event
  // @usage local input = require "ovis.input"
  // local KeyReleaseEvent = input.KeyReleaseEvent
  sol::usertype<KeyReleaseEvent> key_event_type = module->new_usertype<KeyReleaseEvent>(
      "KeyReleaseEvent", sol::no_constructor, sol::base_classes, sol::bases<Event>());

  /// The key that is released.
  // @field[type=Key] key
  key_event_type["key"] = sol::property(&KeyReleaseEvent::key);
}

}  // namespace ovis
