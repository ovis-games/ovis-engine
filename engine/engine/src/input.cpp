#include <ovis/engine/input.hpp>
#include <ovis/engine/lua.hpp>

namespace ovis {

Input::Input() {}

void Input::RegisterToLua() {
  struct InputKeyStates {
    Input* input;
  };

  sol::usertype<Input> input_type = Lua::state.new_usertype<Input>("Input", sol::no_constructor);
  input_type["get_key_state"] = &Input::GetKeyState;

  Lua::state["input"] = ovis::input();
}

Input* input() {
  static Input input;
  return &input;
}

}  // namespace ovis
