#include <ovis/input/mouse_events.hpp>

namespace ovis {

void MouseWheelEvent::RegisterType(sol::table* module) {
  /// This event occures when a mouse button is released.
  // @classmod ovis.input.MouseWheelEvent
  // @base ovis.core.Event
  // @usage local input = require "ovis.input"
  // local MouseWheelEvent = input.MouseWheelEvent
  sol::usertype<MouseWheelEvent> mouse_wheel_event = module->new_usertype<MouseWheelEvent>(
      "MouseWheelEvent", sol::no_constructor, sol::base_classes, sol::bases<Event>());

  /// The amount the scroll wheel was turned.
  // The x component refers to the hrozontal scrolling and the y component to vertical scrolling.
  // Most mice do not have a horizontal scroll wheel so `delta.x` will be zero most of the time.
  // @field[type=ovis.core.Vector2] delta
  mouse_wheel_event["delta"] = sol::property(&MouseWheelEvent::delta);
}

}  // namespace ovis
