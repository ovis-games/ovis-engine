#include <ovis/input/mouse_events.hpp>

namespace ovis {

void MouseMoveEvent::RegisterType(sol::table* module) {
  /// This event occures when the mouse is moved.
  // @classmod ovis.input.MouseMoveEvent
  // @base ovis.core.Event
  // @usage local input = require "ovis.input"
  // local MouseMoveEvent = input.MouseMoveEvent
  sol::usertype<MouseMoveEvent> mouse_move_event = module->new_usertype<MouseMoveEvent>(
      "MouseMoveEvent", sol::no_constructor, sol::base_classes, sol::bases<Event>());

  /// The viewport the mouse event occured on.
  // @field[type=ovis.core.SceneViewport] viewport
  mouse_move_event["viewport"] = sol::property(&MouseMoveEvent::viewport);

  /// The position of the mouse in screen space.
  // @field[type=ovis.core.Vector2] screen_space_position
  mouse_move_event["screen_space_position"] = sol::property(&MouseMoveEvent::screen_space_position);

  /// The delta to the previous postion in screen space.
  // @field[type=ovis.core.Vector2] relative_screen_space_position
  mouse_move_event["relative_screen_space_position"] = sol::property(&MouseMoveEvent::relative_screen_space_position);
}

}  // namespace ovis
