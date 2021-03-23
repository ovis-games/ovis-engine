#include <sol/sol.hpp>

#include <ovis/core/core_module.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/input/input_module.hpp>
#include <ovis/input/key.hpp>
#include <ovis/input/key_events.hpp>
#include <ovis/input/mouse_button.hpp>
#include <ovis/input/mouse_events.hpp>

namespace ovis {

int LoadInputModule(lua_State* l) {
  sol::state_view state(l);

  /// This module provides core components of the engine.
  // @module ovis.input
  // @usage local input = require('ovis.input')
  sol::table input_module = state.create_table();

  Key::RegisterType(&input_module);
  KeyPressEvent::RegisterType(&input_module);
  KeyReleaseEvent::RegisterType(&input_module);

  /// Checks whether the given key is currently pressed.
  // @function get_key_state
  // @param[type=Key] key
  // @return[type=bool] True if the key is currently pressed, false otherwise.
  // @usage input.get_key_state(input.Key.ARROW_LEFT) -- returns true if the left arrow key is pressed
  input_module["get_key_state"] = &GetKeyState;

  MouseButton::RegisterType(&input_module);
  MouseButtonPressEvent::RegisterType(&input_module);
  MouseButtonReleaseEvent::RegisterType(&input_module);
  MouseMoveEvent::RegisterType(&input_module);
  MouseWheelEvent::RegisterType(&input_module);

  /// Checks whether the given mouse button is currently pressed.
  // @function get_mouse_button_state
  // @param[type=MouseButton] button
  // @return[type=bool] True if the button is currently pressed, false otherwise.
  // @usage input.get_mouse_button_state(input.MouseButton.LEFT) -- returns true if the left mouse button is pressed
  input_module["get_mouse_button_state"] = &GetMouseButtonState;

  return input_module.push();
}

bool LoadInputModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    LoadCoreModule();
    lua.require("ovis.input", &LoadInputModule);
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis