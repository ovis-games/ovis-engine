#include <ovis/input/key_event.hpp>

namespace ovis {

void KeyEvent::RegisterType(sol::table* module) {
  /// Indicates that a key was pressed or released.
  // @classmod ovis.input.KeyEvent
  // @base ovis.core.Event
  // @usage local input = require "ovis.input"
  // local KeyEvent = input.KeyEvent
  sol::usertype<KeyEvent> key_event_type = module->new_usertype<KeyEvent>("KeyEvent", sol::no_constructor);

  /// The key corresponding to this event.
  // @field[type=Key] key
  key_event_type["key"] = sol::property(&KeyEvent::key);

  /// Returns true if the key was pressed.
  // @field[type=bool] pressed
  key_event_type["pressed"] = sol::property(&KeyEvent::pressed);

  /// Returns true if the key was released.
  // @field[type=bool] released
  key_event_type["released"] = sol::property(&KeyEvent::released);
}

}  // namespace ovis
