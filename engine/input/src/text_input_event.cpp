#include <ovis/input/text_input_event.hpp>

namespace ovis {

void TextInputEvent::RegisterType(sol::table* module) {
  /// Indicates that a key was pressed or released.
  // @classmod ovis.input.TextInputEvent
  // @base ovis.core.Event
  // @usage local input = require "ovis.input"
  // local TextInputEvent = input.TextInputEvent
  sol::usertype<TextInputEvent> text_input_event_type = module->new_usertype<TextInputEvent>("TextInputEvent", sol::no_constructor);

  /// The text that was input.
  // @field[type=string] text
  text_input_event_type["text"] = sol::property(&TextInputEvent::text);
}

}  // namespace ovis
