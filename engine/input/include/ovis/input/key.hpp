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

  static Key FromName(std::string_view name);

  static constexpr Key Digit1() { return {SDL_SCANCODE_1}; }
  static constexpr Key Digit2() { return {SDL_SCANCODE_2}; }
  static constexpr Key Digit3() { return {SDL_SCANCODE_3}; }
  static constexpr Key Digit4() { return {SDL_SCANCODE_4}; }
  static constexpr Key Digit5() { return {SDL_SCANCODE_5}; }
  static constexpr Key Digit6() { return {SDL_SCANCODE_6}; }
  static constexpr Key Digit7() { return {SDL_SCANCODE_7}; }
  static constexpr Key Digit8() { return {SDL_SCANCODE_8}; }
  static constexpr Key Digit9() { return {SDL_SCANCODE_9}; }
  static constexpr Key Digit0() { return {SDL_SCANCODE_0}; }
  static constexpr Key Minus() { return {SDL_SCANCODE_MINUS}; }
  static constexpr Key Equal() { return {SDL_SCANCODE_EQUALS}; }
  static constexpr Key Numpad0() { return {SDL_SCANCODE_KP_0}; }
  static constexpr Key Numpad1() { return {SDL_SCANCODE_KP_1}; }
  static constexpr Key Numpad2() { return {SDL_SCANCODE_KP_2}; }
  static constexpr Key Numpad3() { return {SDL_SCANCODE_KP_3}; }
  static constexpr Key Numpad4() { return {SDL_SCANCODE_KP_4}; }
  static constexpr Key Numpad5() { return {SDL_SCANCODE_KP_5}; }
  static constexpr Key Numpad6() { return {SDL_SCANCODE_KP_6}; }
  static constexpr Key Numpad7() { return {SDL_SCANCODE_KP_7}; }
  static constexpr Key Numpad8() { return {SDL_SCANCODE_KP_8}; }
  static constexpr Key Numpad9() { return {SDL_SCANCODE_KP_9}; }
  static constexpr Key NumpadDecimal() { return {SDL_SCANCODE_KP_PERIOD}; }
  static constexpr Key NumpadEnter() { return {SDL_SCANCODE_KP_ENTER}; }
  static constexpr Key NumpadAdd() { return {SDL_SCANCODE_KP_PLUS}; }
  static constexpr Key NumpadSubtract() { return {SDL_SCANCODE_KP_MINUS}; }
  static constexpr Key NumpadMultiply() { return {SDL_SCANCODE_KP_MULTIPLY}; }
  static constexpr Key NumpadDivide() { return {SDL_SCANCODE_KP_DIVIDE}; }
  static constexpr Key Escape() { return {SDL_SCANCODE_ESCAPE}; }
  static constexpr Key Insert() { return {SDL_SCANCODE_INSERT}; }
  static constexpr Key Delete() { return {SDL_SCANCODE_DELETE}; }
  static constexpr Key Home() { return {SDL_SCANCODE_HOME}; }
  static constexpr Key End() { return {SDL_SCANCODE_END}; }
  static constexpr Key PageUp() { return {SDL_SCANCODE_PAGEUP}; }
  static constexpr Key PageDown() { return {SDL_SCANCODE_PAGEDOWN}; }
  static constexpr Key Tab() { return {SDL_SCANCODE_TAB}; }
  static constexpr Key Enter() { return {SDL_SCANCODE_RETURN}; }
  static constexpr Key Backspace() { return {SDL_SCANCODE_BACKSPACE}; }
  static constexpr Key ControlLeft() { return {SDL_SCANCODE_LCTRL}; }
  static constexpr Key ControlRight() { return {SDL_SCANCODE_RCTRL}; }
  static constexpr Key AltLeft() { return {SDL_SCANCODE_LALT}; }
  static constexpr Key AltRight() { return {SDL_SCANCODE_RALT}; }
  static constexpr Key MetaLeft() { return {SDL_SCANCODE_LGUI}; }
  static constexpr Key MetaRight() { return {SDL_SCANCODE_RGUI}; }
  static constexpr Key ShiftLeft() { return {SDL_SCANCODE_LSHIFT}; }
  static constexpr Key ShiftRight() { return {SDL_SCANCODE_RSHIFT}; }
  static constexpr Key ArrowUp() { return {SDL_SCANCODE_UP}; }
  static constexpr Key ArrowDown() { return {SDL_SCANCODE_DOWN}; }
  static constexpr Key ArrowLeft() { return {SDL_SCANCODE_LEFT}; }
  static constexpr Key ArrowRight() { return {SDL_SCANCODE_RIGHT}; }
  static constexpr Key Backquote() { return {SDL_SCANCODE_GRAVE}; }
  static constexpr Key BracketLeft() { return {SDL_SCANCODE_LEFTBRACKET}; }
  static constexpr Key BracketRight() { return {SDL_SCANCODE_RIGHTBRACKET}; }
  static constexpr Key Semicolon() { return {SDL_SCANCODE_SEMICOLON}; }
  static constexpr Key Quote() { return {SDL_SCANCODE_APOSTROPHE}; }
  static constexpr Key Backslash() { return {SDL_SCANCODE_BACKSLASH}; }
  static constexpr Key Comma() { return {SDL_SCANCODE_COMMA}; }
  static constexpr Key Period() { return {SDL_SCANCODE_PERIOD}; }
  static constexpr Key Slash() { return {SDL_SCANCODE_SLASH}; }
  static constexpr Key IntlBackslash() { return {SDL_SCANCODE_NONUSBACKSLASH}; }
  static constexpr Key Space() { return {SDL_SCANCODE_SPACE}; }
  static constexpr Key A() { return {SDL_SCANCODE_A}; }
  static constexpr Key B() { return {SDL_SCANCODE_B}; }
  static constexpr Key C() { return {SDL_SCANCODE_C}; }
  static constexpr Key D() { return {SDL_SCANCODE_D}; }
  static constexpr Key E() { return {SDL_SCANCODE_E}; }
  static constexpr Key F() { return {SDL_SCANCODE_F}; }
  static constexpr Key G() { return {SDL_SCANCODE_G}; }
  static constexpr Key H() { return {SDL_SCANCODE_H}; }
  static constexpr Key I() { return {SDL_SCANCODE_I}; }
  static constexpr Key J() { return {SDL_SCANCODE_J}; }
  static constexpr Key K() { return {SDL_SCANCODE_K}; }
  static constexpr Key L() { return {SDL_SCANCODE_L}; }
  static constexpr Key M() { return {SDL_SCANCODE_M}; }
  static constexpr Key N() { return {SDL_SCANCODE_N}; }
  static constexpr Key O() { return {SDL_SCANCODE_O}; }
  static constexpr Key P() { return {SDL_SCANCODE_P}; }
  static constexpr Key Q() { return {SDL_SCANCODE_Q}; }
  static constexpr Key R() { return {SDL_SCANCODE_R}; }
  static constexpr Key S() { return {SDL_SCANCODE_S}; }
  static constexpr Key T() { return {SDL_SCANCODE_T}; }
  static constexpr Key U() { return {SDL_SCANCODE_U}; }
  static constexpr Key V() { return {SDL_SCANCODE_V}; }
  static constexpr Key W() { return {SDL_SCANCODE_W}; }
  static constexpr Key X() { return {SDL_SCANCODE_X}; }
  static constexpr Key Y() { return {SDL_SCANCODE_Y}; }
  static constexpr Key Z() { return {SDL_SCANCODE_Z}; }
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
static_assert(sizeof(Key) == sizeof(Key::KeyCode));

// TODO: don't store the keys in a vector as we already have them in a map. Instead of storing them twice adapt the map
// iterators.
const std::vector<Key>& Keys();

inline bool operator==(Key lhs, Key rhs) {
  return lhs.code == rhs.code;
}

inline bool operator!=(Key lhs, Key rhs) {
  return lhs.code != rhs.code;
}

bool GetKeyState(Key key);
void SetKeyState(Key key, bool pressed);

}  // namespace ovis
