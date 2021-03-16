#pragma once

#include <cstdint>
#include <limits>
#include <string_view>

#include <SDL2/SDL_scancode.h>
#include <sol/sol.hpp>

namespace ovis {

struct Key {
  using KeyCode = uint16_t;
  KeyCode code;
  static_assert(std::numeric_limits<KeyCode>::max() >= SDL_NUM_SCANCODES);

  std::string_view id() const;
  std::string_view name() const;

  static Key FromName(const std::string& name);

  static constexpr Key DIGIT_1() { return {SDL_SCANCODE_1}; }
  static constexpr Key DIGIT_2() { return {SDL_SCANCODE_2}; }
  static constexpr Key DIGIT_3() { return {SDL_SCANCODE_3}; }
  static constexpr Key DIGIT_4() { return {SDL_SCANCODE_4}; }
  static constexpr Key DIGIT_5() { return {SDL_SCANCODE_5}; }
  static constexpr Key DIGIT_6() { return {SDL_SCANCODE_6}; }
  static constexpr Key DIGIT_7() { return {SDL_SCANCODE_7}; }
  static constexpr Key DIGIT_8() { return {SDL_SCANCODE_8}; }
  static constexpr Key DIGIT_9() { return {SDL_SCANCODE_9}; }
  static constexpr Key DIGIT_0() { return {SDL_SCANCODE_0}; }
  static constexpr Key MINUS() { return {SDL_SCANCODE_MINUS}; }
  static constexpr Key EQUAL() { return {SDL_SCANCODE_EQUALS}; }
  static constexpr Key NUMPAD_0() { return {SDL_SCANCODE_KP_0}; }
  static constexpr Key NUMPAD_1() { return {SDL_SCANCODE_KP_1}; }
  static constexpr Key NUMPAD_2() { return {SDL_SCANCODE_KP_2}; }
  static constexpr Key NUMPAD_3() { return {SDL_SCANCODE_KP_3}; }
  static constexpr Key NUMPAD_4() { return {SDL_SCANCODE_KP_4}; }
  static constexpr Key NUMPAD_5() { return {SDL_SCANCODE_KP_5}; }
  static constexpr Key NUMPAD_6() { return {SDL_SCANCODE_KP_6}; }
  static constexpr Key NUMPAD_7() { return {SDL_SCANCODE_KP_7}; }
  static constexpr Key NUMPAD_8() { return {SDL_SCANCODE_KP_8}; }
  static constexpr Key NUMPAD_9() { return {SDL_SCANCODE_KP_9}; }
  static constexpr Key NUMPAD_DECIMAL() { return {SDL_SCANCODE_KP_PERIOD}; }
  static constexpr Key NUMPAD_ENTER() { return {SDL_SCANCODE_KP_ENTER}; }
  static constexpr Key NUMPAD_ADD() { return {SDL_SCANCODE_KP_PLUS}; }
  static constexpr Key NUMPAD_SUBTRACT() { return {SDL_SCANCODE_KP_MINUS}; }
  static constexpr Key NUMPAD_MULTIPLY() { return {SDL_SCANCODE_KP_MULTIPLY}; }
  static constexpr Key NUMPAD_DIVIDE() { return {SDL_SCANCODE_KP_DIVIDE}; }
  static constexpr Key ESCAPE() { return {SDL_SCANCODE_ESCAPE}; }
  static constexpr Key INSERT() { return {SDL_SCANCODE_INSERT}; }
  static constexpr Key DELETE() { return {SDL_SCANCODE_DELETE}; }
  static constexpr Key HOME() { return {SDL_SCANCODE_HOME}; }
  static constexpr Key END() { return {SDL_SCANCODE_END}; }
  static constexpr Key PAGE_UP() { return {SDL_SCANCODE_PAGEUP}; }
  static constexpr Key PAGE_DOWN() { return {SDL_SCANCODE_PAGEDOWN}; }
  static constexpr Key TAB() { return {SDL_SCANCODE_TAB}; }
  static constexpr Key ENTER() { return {SDL_SCANCODE_RETURN}; }
  static constexpr Key BACKSPACE() { return {SDL_SCANCODE_BACKSPACE}; }
  static constexpr Key CONTROL_LEFT() { return {SDL_SCANCODE_LCTRL}; }
  static constexpr Key CONTROL_RIGHT() { return {SDL_SCANCODE_RCTRL}; }
  static constexpr Key ALT_LEFT() { return {SDL_SCANCODE_LALT}; }
  static constexpr Key ALT_RIGHT() { return {SDL_SCANCODE_RALT}; }
  static constexpr Key META_LEFT() { return {SDL_SCANCODE_LGUI}; }
  static constexpr Key META_RIGHT() { return {SDL_SCANCODE_RGUI}; }
  static constexpr Key SHIFT_LEFT() { return {SDL_SCANCODE_LSHIFT}; }
  static constexpr Key SHIFT_RIGHT() { return {SDL_SCANCODE_RSHIFT}; }
  static constexpr Key ARROW_UP() { return {SDL_SCANCODE_UP}; }
  static constexpr Key ARROW_DOWN() { return {SDL_SCANCODE_DOWN}; }
  static constexpr Key ARROW_LEFT() { return {SDL_SCANCODE_LEFT}; }
  static constexpr Key ARROW_RIGHT() { return {SDL_SCANCODE_RIGHT}; }
  static constexpr Key BACKQUOTE() { return {SDL_SCANCODE_GRAVE}; }
  static constexpr Key BRACKET_LEFT() { return {SDL_SCANCODE_LEFTBRACKET}; }
  static constexpr Key BRACKET_RIGHT() { return {SDL_SCANCODE_RIGHTBRACKET}; }
  static constexpr Key SEMICOLON() { return {SDL_SCANCODE_SEMICOLON}; }
  static constexpr Key QUOTE() { return {SDL_SCANCODE_APOSTROPHE}; }
  static constexpr Key BACKSLASH() { return {SDL_SCANCODE_BACKSLASH}; }
  static constexpr Key COMMA() { return {SDL_SCANCODE_COMMA}; }
  static constexpr Key PERIOD() { return {SDL_SCANCODE_PERIOD}; }
  static constexpr Key SLASH() { return {SDL_SCANCODE_SLASH}; }
  static constexpr Key INTL_BACKSLASH() { return {SDL_SCANCODE_NONUSBACKSLASH}; }
  static constexpr Key SPACE() { return {SDL_SCANCODE_SPACE}; }
  static constexpr Key KEY_A() { return {SDL_SCANCODE_A}; }
  static constexpr Key KEY_B() { return {SDL_SCANCODE_B}; }
  static constexpr Key KEY_C() { return {SDL_SCANCODE_C}; }
  static constexpr Key KEY_D() { return {SDL_SCANCODE_D}; }
  static constexpr Key KEY_E() { return {SDL_SCANCODE_E}; }
  static constexpr Key KEY_F() { return {SDL_SCANCODE_F}; }
  static constexpr Key KEY_G() { return {SDL_SCANCODE_G}; }
  static constexpr Key KEY_H() { return {SDL_SCANCODE_H}; }
  static constexpr Key KEY_I() { return {SDL_SCANCODE_I}; }
  static constexpr Key KEY_J() { return {SDL_SCANCODE_J}; }
  static constexpr Key KEY_K() { return {SDL_SCANCODE_K}; }
  static constexpr Key KEY_L() { return {SDL_SCANCODE_L}; }
  static constexpr Key KEY_M() { return {SDL_SCANCODE_M}; }
  static constexpr Key KEY_N() { return {SDL_SCANCODE_N}; }
  static constexpr Key KEY_O() { return {SDL_SCANCODE_O}; }
  static constexpr Key KEY_P() { return {SDL_SCANCODE_P}; }
  static constexpr Key KEY_Q() { return {SDL_SCANCODE_Q}; }
  static constexpr Key KEY_R() { return {SDL_SCANCODE_R}; }
  static constexpr Key KEY_S() { return {SDL_SCANCODE_S}; }
  static constexpr Key KEY_T() { return {SDL_SCANCODE_T}; }
  static constexpr Key KEY_U() { return {SDL_SCANCODE_U}; }
  static constexpr Key KEY_V() { return {SDL_SCANCODE_V}; }
  static constexpr Key KEY_W() { return {SDL_SCANCODE_W}; }
  static constexpr Key KEY_X() { return {SDL_SCANCODE_X}; }
  static constexpr Key KEY_Y() { return {SDL_SCANCODE_Y}; }
  static constexpr Key KEY_Z() { return {SDL_SCANCODE_Z}; }
  static constexpr Key F1() { return {SDL_SCANCODE_F1}; }
  static constexpr Key F2() { return {SDL_SCANCODE_F2}; }
  static constexpr Key F3() { return {SDL_SCANCODE_F3}; }
  static constexpr Key F4() { return {SDL_SCANCODE_F4}; }
  static constexpr Key F5() { return {SDL_SCANCODE_F5}; }
  static constexpr Key F6() { return {SDL_SCANCODE_F6}; }
  static constexpr Key F7() { return {SDL_SCANCODE_F7}; }
  static constexpr Key F8() { return {SDL_SCANCODE_F8}; }
  static constexpr Key F9() { return {SDL_SCANCODE_F9}; }
  static constexpr Key F10() { return {SDL_SCANCODE_F10}; }
  static constexpr Key F11() { return {SDL_SCANCODE_F11}; }
  static constexpr Key F12() { return {SDL_SCANCODE_F12}; }
  static constexpr Key F13() { return {SDL_SCANCODE_F13}; }
  static constexpr Key F14() { return {SDL_SCANCODE_F14}; }
  static constexpr Key F15() { return {SDL_SCANCODE_F15}; }
  static constexpr Key F16() { return {SDL_SCANCODE_F16}; }
  static constexpr Key F17() { return {SDL_SCANCODE_F17}; }
  static constexpr Key F18() { return {SDL_SCANCODE_F18}; }
  static constexpr Key F19() { return {SDL_SCANCODE_F19}; }
  static constexpr Key F20() { return {SDL_SCANCODE_F20}; }
  static constexpr Key F21() { return {SDL_SCANCODE_F21}; }
  static constexpr Key F22() { return {SDL_SCANCODE_F22}; }
  static constexpr Key F23() { return {SDL_SCANCODE_F23}; }
  static constexpr Key F24() { return {SDL_SCANCODE_F24}; }

  static void RegisterType(sol::table* module);
};
static_assert(sizeof(Key) == sizeof(KeyCode));

inline bool operator==(Key lhs, Key rhs) {
  return lhs.code == rhs.code;
}

inline bool operator!=(Key lhs, Key rhs) {
  return lhs.code != rhs.code;
}

}  // namespace ovis
