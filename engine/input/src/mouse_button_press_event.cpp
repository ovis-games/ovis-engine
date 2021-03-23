#include <ovis/input/mouse_events.hpp>

namespace ovis {

void MouseButtonPressEvent::RegisterType(sol::table* module) {
  /// This event occures when a mouse button is pressed.
  // @classmod ovis.input.MouseButtonPressEvent
  // @base ovis.core.Event
  // @usage local input = require "ovis.input"
  // local MouseButtonPressEvent = input.MouseButtonPressEvent
  sol::usertype<MouseButtonPressEvent> mouse_button_press_event =
      module->new_usertype<MouseButtonPressEvent>("MouseButtonPressEvent", sol::no_constructor, sol::base_classes, sol::bases<Event>());

  /// The viewport the mouse event occured on.
  // @field[type=ovis.core.SceneViewport] viewport
  mouse_button_press_event["viewport"] = sol::property(&MouseButtonPressEvent::viewport);

  /// The position of the mouse in screen space.
  // @field[type=ovis.core.Vector2] screen_space_position
  mouse_button_press_event["screen_space_position"] = sol::property(&MouseButtonPressEvent::screen_space_position);

  /// The button that was pressed.
  // @field[type=MouseButton] button
  mouse_button_press_event["button"] = sol::property(&MouseButtonPressEvent::button);
}

}  // namespace ovis
