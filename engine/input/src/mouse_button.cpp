#include <unordered_map>

#include <SDL2/SDL_assert.h>

#include <ovis/input/mouse_button.hpp>

namespace ovis {

namespace {
static const std::unordered_map<std::string_view, uint8_t> BUTTON_NAME_TO_BUTTON = {
  // clang-format off
  {"Left Mouse Button", 0},
  {"Middle Mouse Button", 1},
  {"Right Mouse Button", 2},
  {"Mouse Button 4", 3},
  {"Mouse Button 5", 4},
  // clang-format on
};
}

std::string_view MouseButton::id() const {
  // clang-format off
  switch (code) {
    case Left().code: return "LEFT";
    case Middle().code: return "MIDDLE";
    case Right().code: return "RIGHT";
    case Four().code: return "FOUR";
    case Five().code: return "FIVE";
    default: SDL_assert(false);  return "";
  }
  // clang-format on
}

std::string_view MouseButton::name() const {
  // clang-format off
  switch (code) {
    case Left().code: return "Left Mouse Button";
    case Middle().code: return "Middle Mouse Button";
    case Right().code: return "Right Mouse Button";
    case Four().code: return "Mouse Button 4";
    case Five().code: return "Mouse Button 5";
    default: SDL_assert(false);  return "";
  }
  // clang-format on
}

MouseButton MouseButton::FromName(std::string_view name) {
  return {BUTTON_NAME_TO_BUTTON.at(name)};
}

void MouseButton::RegisterType(sol::table* module) {
  /// Represents a key on a keyboard.
  // @classmod ovis.input.MouseButton
  // @usage local input = require "ovis.input"
  // local MouseButton = input.MouseButton
  sol::usertype<MouseButton> key_type = module->new_usertype<MouseButton>("MouseButton", sol::no_constructor);

  /// Returns a unique, human readable name for a key.
  // @field[type=string] name
  // @usage assert(MouseButton.NUMPAD_0.name == "Numpad 0")
  key_type["name"] = sol::property(&MouseButton::name);

  /// Creates a key from its human readable name.
  // @function from_name
  // @param[type=string] name
  // @usage assert(MouseButton.from_name("Numpad 0") == MouseButton.NUMPAD_0)
  key_type["from_name"] = &MouseButton::FromName;

  /// Compares two keys
  // @function __eq
  // @param[type=MouseButton] k1
  // @param[type=MouseButton] k2
  // @treturn bool
  // @usage local numpad_zero = MouseButton.NUMPAD_0
  // assert(numpad_zero == MouseButton.NUMPAD_0)
  // assert(numpad_zero ~= MouseButton.NUMPAD_1)
  key_type[sol::meta_function::equal_to] = static_cast<bool (*)(MouseButton, MouseButton)>(ovis::operator==);

  /// Returns a unique, human readable name for a key.
  // @see name
  // @function __tostring
  // @treturn string
  // @usage assert(tostring(MouseButton.NUMPAD_0) == MouseButton.NUMPAD_0.name)
  key_type[sol::meta_function::to_string] = &MouseButton::name;

  /// LEFT.
  // @field[type=MouseButton] LEFT
  key_type["LEFT"] = sol::property(&MouseButton::Left);
  /// MIDDLE.
  // @field[type=MouseButton] MIDDLE
  key_type["MIDDLE"] = sol::property(&MouseButton::Middle);
  /// RIGHT.
  // @field[type=MouseButton] RIGHT
  key_type["RIGHT"] = sol::property(&MouseButton::Right);
  /// FOUR.
  // @field[type=MouseButton] FOUR
  key_type["FOUR"] = sol::property(&MouseButton::Four);
  /// FIVE.
  // @field[type=MouseButton] FIVE
  key_type["FIVE"] = sol::property(&MouseButton::Five);
}

namespace {
bool mouse_button_states[5] = {false};
}

bool GetMouseButtonState(MouseButton button) {
  SDL_assert(button.code < 5);
  return mouse_button_states[button.code];
}

void SetMouseButtonState(MouseButton button, bool pressed) {
  SDL_assert(button.code < 5);
  mouse_button_states[button.code] = pressed;
}

}  // namespace ovis
