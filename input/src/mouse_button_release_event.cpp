#include <ovis/input/mouse_events.hpp>

namespace ovis {

void MouseButtonReleaseEvent::RegisterType(sol::table* module) {
  /// This event occures when a mouse button is released.
  // @classmod ovis.input.MouseButtonReleaseEvent
  // @base ovis.core.Event
  // @usage local input = require "ovis.input"
  // local MouseButtonReleaseEvent = input.MouseButtonReleaseEvent
  sol::usertype<MouseButtonReleaseEvent> mouse_button_release_event = module->new_usertype<MouseButtonReleaseEvent>(
      "MouseButtonReleaseEvent", sol::no_constructor, sol::base_classes, sol::bases<Event>());

  /// The viewport the mouse event occured on.
  // @field[type=ovis.core.SceneViewport] viewport
  mouse_button_release_event["viewport"] = sol::property(&MouseButtonReleaseEvent::viewport);

  /// The position of the mouse in screen space.
  // @field[type=ovis.core.Vector2] screen_space_position
  mouse_button_release_event["screen_space_position"] = sol::property(&MouseButtonReleaseEvent::screen_space_position);

  /// The button that was released.
  // @field[type=MouseButton] button
  mouse_button_release_event["button"] = sol::property(&MouseButtonReleaseEvent::button);
}

}  // namespace ovis
