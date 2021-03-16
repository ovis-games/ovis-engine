#include <ovis/input/input.hpp>

namespace ovis {

Input::Input() {}

void Input::RegisterType(sol::table* module) {
  sol::state_view lua(module->lua_state());

  // clang-format off

  /// Input submodule.
  // @module ovis.engine.input
  // @usage local engine = require "ovis.engine"
  // input = engine.input
  sol::table input = lua.create_table();

  /// Mouse Buttons.
  // @table MouseButton
  input.new_enum("MouseButton",
    /// Left mouse button
    // @field[type=MouseButton] LEFT
    "LEFT", MouseButton::LEFT,
    /// Middle mouse button
    // @field[type=MouseButton] MIDDLE
    "MIDDLE", MouseButton::MIDDLE,
    /// Right mouse button
    // @field[type=MouseButton] RIGHT
    "RIGHT", MouseButton::RIGHT,
    /// Extra mouse button 1 ("back")
    // @field[type=MouseButton] EXTRA1
    "EXTRA1", MouseButton::EXTRA1,
    /// Extra mouse button 2 ("forward")
    // @field[type=MouseButton] EXTRA2
    "EXTRA2", MouseButton::EXTRA2
  );

  /// Checks whether a mouse button is pressed.
  // @function get_mouse_button_state
  // @param key Key
  // @return bool
  // @usage local core = require "ovis.core"
  // if input.get_mouse_button_state("") then
  //  core.log("The left mouse button is pressed")
  // else
  //  core.log("The left mouse button is not pressed")
  // end
  input["get_mouse_button_state"] = [](MouseButton button) {
    return ovis::input()->GetMouseButtonState(button);
  };


  /// Checks whether a key is pressed on the keyboard.
  // @function get_key_state
  // @param key Key
  // @return bool
  // @usage local core = require "ovis.core"
  // if input.get_key_state(5) then
  //  core.log("The key is pressed")
  // else
  //  core.log("The key is not pressed")
  // end
  input["get_key_state"] = [](Key key) {
    return ovis::input()->GetKeyState(key);
  };

  (*module)["input"] = input;

  // sol::usertype<Input> input_type = module->new_usertype<Input>("Input", sol::no_constructor);
  // input_type["get_key_state"] = &Input::GetKeyState;

  // Lua::state["input"] = ovis::input();

  // clang-format on
}

Input* input() {
  static Input input;
  return &input;
}

}  // namespace ovis
