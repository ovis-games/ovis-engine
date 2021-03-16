#include <unordered_map>

#include <ovis/input/keyboard.hpp>

namespace ovis {

namespace {
static const std::unordered_map<std::string, KeyCode> KEY_NAME_TO_KEY = {
    {"Digit1", SDL_SCANCODE_1},
    {"Digit2", SDL_SCANCODE_2},
    {"Digit3", SDL_SCANCODE_3},
    {"Digit4", SDL_SCANCODE_4},
    {"Digit5", SDL_SCANCODE_5},
    {"Digit6", SDL_SCANCODE_6},
    {"Digit7", SDL_SCANCODE_7},
    {"Digit8", SDL_SCANCODE_8},
    {"Digit9", SDL_SCANCODE_9},
    {"Digit0", SDL_SCANCODE_0},
    {"Minus", SDL_SCANCODE_MINUS},
    {"Equal", SDL_SCANCODE_EQUALS},
    {"Numpad0", SDL_SCANCODE_KP_0},
    {"Numpad1", SDL_SCANCODE_KP_1},
    {"Numpad2", SDL_SCANCODE_KP_2},
    {"Numpad3", SDL_SCANCODE_KP_3},
    {"Numpad4", SDL_SCANCODE_KP_4},
    {"Numpad5", SDL_SCANCODE_KP_5},
    {"Numpad6", SDL_SCANCODE_KP_6},
    {"Numpad7", SDL_SCANCODE_KP_7},
    {"Numpad8", SDL_SCANCODE_KP_8},
    {"Numpad9", SDL_SCANCODE_KP_9},
    {"NumpadDecimal", SDL_SCANCODE_KP_PERIOD},
    {"NumpadEnter", SDL_SCANCODE_KP_ENTER},
    {"NumpadAdd", SDL_SCANCODE_KP_PLUS},
    {"NumpadSubtract", SDL_SCANCODE_KP_MINUS},
    {"NumpadMultiply", SDL_SCANCODE_KP_MULTIPLY},
    {"NumpadDivide", SDL_SCANCODE_KP_DIVIDE},
    {"Escape", SDL_SCANCODE_ESCAPE},
    {"Insert", SDL_SCANCODE_INSERT},
    {"Delete", SDL_SCANCODE_DELETE},
    {"Home", SDL_SCANCODE_HOME},
    {"End", SDL_SCANCODE_END},
    {"PageUp", SDL_SCANCODE_PAGEUP},
    {"PageDown", SDL_SCANCODE_PAGEDOWN},
    {"Tab", SDL_SCANCODE_TAB},
    {"Enter", SDL_SCANCODE_RETURN},
    {"Backspace", SDL_SCANCODE_BACKSPACE},
    {"ControlLeft", SDL_SCANCODE_LCTRL},
    {"ControlRight", SDL_SCANCODE_RCTRL},
    {"AltLeft", SDL_SCANCODE_LALT},
    {"AltRight", SDL_SCANCODE_RALT},
    {"MetaLeft", SDL_SCANCODE_LGUI},
    {"MetaRight", SDL_SCANCODE_RGUI},
    {"ShiftLeft", SDL_SCANCODE_LSHIFT},
    {"ShiftRight", SDL_SCANCODE_RSHIFT},
    {"ArrowUp", SDL_SCANCODE_UP},
    {"ArrowDown", SDL_SCANCODE_DOWN},
    {"ArrowLeft", SDL_SCANCODE_LEFT},
    {"ArrowRight", SDL_SCANCODE_RIGHT},
    {"Backquote", SDL_SCANCODE_GRAVE},
    {"BracketLeft", SDL_SCANCODE_LEFTBRACKET},
    {"BracketRight", SDL_SCANCODE_RIGHTBRACKET},
    {"Semicolon", SDL_SCANCODE_SEMICOLON},
    {"Quote", SDL_SCANCODE_APOSTROPHE},
    {"Backslash", SDL_SCANCODE_BACKSLASH},
    {"Comma", SDL_SCANCODE_COMMA},
    {"Period", SDL_SCANCODE_PERIOD},
    {"Slash", SDL_SCANCODE_SLASH},
    {"IntlBackslash", SDL_SCANCODE_NONUSBACKSLASH},
    {"Space", SDL_SCANCODE_SPACE},
    {"KeyA", SDL_SCANCODE_A},
    {"KeyB", SDL_SCANCODE_B},
    {"KeyC", SDL_SCANCODE_C},
    {"KeyD", SDL_SCANCODE_D},
    {"KeyE", SDL_SCANCODE_E},
    {"KeyF", SDL_SCANCODE_F},
    {"KeyG", SDL_SCANCODE_G},
    {"KeyH", SDL_SCANCODE_H},
    {"KeyI", SDL_SCANCODE_I},
    {"KeyJ", SDL_SCANCODE_J},
    {"KeyK", SDL_SCANCODE_K},
    {"KeyL", SDL_SCANCODE_L},
    {"KeyM", SDL_SCANCODE_M},
    {"KeyN", SDL_SCANCODE_N},
    {"KeyO", SDL_SCANCODE_O},
    {"KeyP", SDL_SCANCODE_P},
    {"KeyQ", SDL_SCANCODE_Q},
    {"KeyR", SDL_SCANCODE_R},
    {"KeyS", SDL_SCANCODE_S},
    {"KeyT", SDL_SCANCODE_T},
    {"KeyU", SDL_SCANCODE_U},
    {"KeyV", SDL_SCANCODE_V},
    {"KeyW", SDL_SCANCODE_W},
    {"KeyX", SDL_SCANCODE_X},
    {"KeyY", SDL_SCANCODE_Y},
    {"KeyZ", SDL_SCANCODE_Z},
    {"F1", SDL_SCANCODE_F1},
    {"F2", SDL_SCANCODE_F2},
    {"F3", SDL_SCANCODE_F3},
    {"F4", SDL_SCANCODE_F4},
    {"F5", SDL_SCANCODE_F5},
    {"F6", SDL_SCANCODE_F6},
    {"F7", SDL_SCANCODE_F7},
    {"F8", SDL_SCANCODE_F8},
    {"F9", SDL_SCANCODE_F9},
    {"F10", SDL_SCANCODE_F10},
    {"F11", SDL_SCANCODE_F11},
    {"F12", SDL_SCANCODE_F12},
    {"F13", SDL_SCANCODE_F13},
    {"F14", SDL_SCANCODE_F14},
    {"F15", SDL_SCANCODE_F15},
    {"F16", SDL_SCANCODE_F16},
    {"F17", SDL_SCANCODE_F17},
    {"F18", SDL_SCANCODE_F18},
    {"F19", SDL_SCANCODE_F19},
    {"F20", SDL_SCANCODE_F20},
    {"F21", SDL_SCANCODE_F21},
    {"F22", SDL_SCANCODE_F22},
    {"F23", SDL_SCANCODE_F23},
    {"F24", SDL_SCANCODE_F24},
};
}

std::string_view Key::id() const {
  // clang-format off
  switch (code) {
    case SDL_SCANCODE_1: return "DIGIT_1";
    case SDL_SCANCODE_2: return "DIGIT_2";
    case SDL_SCANCODE_3: return "DIGIT_3";
    case SDL_SCANCODE_4: return "DIGIT_4";
    case SDL_SCANCODE_5: return "DIGIT_5";
    case SDL_SCANCODE_6: return "DIGIT_6";
    case SDL_SCANCODE_7: return "DIGIT_7";
    case SDL_SCANCODE_8: return "DIGIT_8";
    case SDL_SCANCODE_9: return "DIGIT_9";
    case SDL_SCANCODE_0: return "DIGIT_0";
    case SDL_SCANCODE_MINUS: return "MINUS";
    case SDL_SCANCODE_EQUALS: return "EQUAL";
    case SDL_SCANCODE_KP_0: return "NUMPAD_0";
    case SDL_SCANCODE_KP_1: return "NUMPAD_1";
    case SDL_SCANCODE_KP_2: return "NUMPAD_2";
    case SDL_SCANCODE_KP_3: return "NUMPAD_3";
    case SDL_SCANCODE_KP_4: return "NUMPAD_4";
    case SDL_SCANCODE_KP_5: return "NUMPAD_5";
    case SDL_SCANCODE_KP_6: return "NUMPAD_6";
    case SDL_SCANCODE_KP_7: return "NUMPAD_7";
    case SDL_SCANCODE_KP_8: return "NUMPAD_8";
    case SDL_SCANCODE_KP_9: return "NUMPAD_9";
    case SDL_SCANCODE_KP_PERIOD: return "NUMPAD_DECIMAL";
    case SDL_SCANCODE_KP_ENTER: return "NUMPAD_ENTER";
    case SDL_SCANCODE_KP_PLUS: return "NUMPAD_ADD";
    case SDL_SCANCODE_KP_MINUS: return "NUMPAD_SUBTRACT";
    case SDL_SCANCODE_KP_MULTIPLY: return "NUMPAD_MULTIPLY";
    case SDL_SCANCODE_KP_DIVIDE: return "NUMPAD_DIVIDE";
    case SDL_SCANCODE_ESCAPE: return "ESCAPE";
    case SDL_SCANCODE_INSERT: return "INSERT";
    case SDL_SCANCODE_DELETE: return "DELETE";
    case SDL_SCANCODE_HOME: return "HOME";
    case SDL_SCANCODE_END: return "END";
    case SDL_SCANCODE_PAGEUP: return "PAGE_UP";
    case SDL_SCANCODE_PAGEDOWN: return "PAGE_DOWN";
    case SDL_SCANCODE_TAB: return "TAB";
    case SDL_SCANCODE_RETURN: return "ENTER";
    case SDL_SCANCODE_BACKSPACE: return "BACKSPACE";
    case SDL_SCANCODE_LCTRL: return "CONTROL_LEFT";
    case SDL_SCANCODE_RCTRL: return "CONTROL_RIGHT";
    case SDL_SCANCODE_LALT: return "ALT_LEFT";
    case SDL_SCANCODE_RALT: return "ALT_RIGHT";
    case SDL_SCANCODE_LGUI: return "META_LEFT";
    case SDL_SCANCODE_RGUI: return "META_RIGHT";
    case SDL_SCANCODE_LSHIFT: return "SHIFT_LEFT";
    case SDL_SCANCODE_RSHIFT: return "SHIFT_RIGHT";
    case SDL_SCANCODE_UP: return "ARROW_UP";
    case SDL_SCANCODE_DOWN: return "ARROW_DOWN";
    case SDL_SCANCODE_LEFT: return "ARROW_LEFT";
    case SDL_SCANCODE_RIGHT: return "ARROW_RIGHT";
    case SDL_SCANCODE_GRAVE: return "BACKQUOTE";
    case SDL_SCANCODE_LEFTBRACKET: return "BRACKET_LEFT";
    case SDL_SCANCODE_RIGHTBRACKET: return "BRACKET_RIGHT";
    case SDL_SCANCODE_SEMICOLON: return "SEMICOLON";
    case SDL_SCANCODE_APOSTROPHE: return "QUOTE";
    case SDL_SCANCODE_BACKSLASH: return "BACKSLASH";
    case SDL_SCANCODE_COMMA: return "COMMA";
    case SDL_SCANCODE_PERIOD: return "PERIOD";
    case SDL_SCANCODE_SLASH: return "SLASH";
    case SDL_SCANCODE_NONUSBACKSLASH: return "INTL_BACKSLASH";
    case SDL_SCANCODE_SPACE: return "SPACE";
    case SDL_SCANCODE_A: return "KEY_A";
    case SDL_SCANCODE_B: return "KEY_B";
    case SDL_SCANCODE_C: return "KEY_C";
    case SDL_SCANCODE_D: return "KEY_D";
    case SDL_SCANCODE_E: return "KEY_E";
    case SDL_SCANCODE_F: return "KEY_F";
    case SDL_SCANCODE_G: return "KEY_G";
    case SDL_SCANCODE_H: return "KEY_H";
    case SDL_SCANCODE_I: return "KEY_I";
    case SDL_SCANCODE_J: return "KEY_J";
    case SDL_SCANCODE_K: return "KEY_K";
    case SDL_SCANCODE_L: return "KEY_L";
    case SDL_SCANCODE_M: return "KEY_M";
    case SDL_SCANCODE_N: return "KEY_N";
    case SDL_SCANCODE_O: return "KEY_O";
    case SDL_SCANCODE_P: return "KEY_P";
    case SDL_SCANCODE_Q: return "KEY_Q";
    case SDL_SCANCODE_R: return "KEY_R";
    case SDL_SCANCODE_S: return "KEY_S";
    case SDL_SCANCODE_T: return "KEY_T";
    case SDL_SCANCODE_U: return "KEY_U";
    case SDL_SCANCODE_V: return "KEY_V";
    case SDL_SCANCODE_W: return "KEY_W";
    case SDL_SCANCODE_X: return "KEY_X";
    case SDL_SCANCODE_Y: return "KEY_Y";
    case SDL_SCANCODE_Z: return "KEY_Z";
    case SDL_SCANCODE_F1: return "F1";
    case SDL_SCANCODE_F2: return "F2";
    case SDL_SCANCODE_F3: return "F3";
    case SDL_SCANCODE_F4: return "F4";
    case SDL_SCANCODE_F5: return "F5";
    case SDL_SCANCODE_F6: return "F6";
    case SDL_SCANCODE_F7: return "F7";
    case SDL_SCANCODE_F8: return "F8";
    case SDL_SCANCODE_F9: return "F9";
    case SDL_SCANCODE_F10: return "F10";
    case SDL_SCANCODE_F11: return "F11";
    case SDL_SCANCODE_F12: return "F12";
    case SDL_SCANCODE_F13: return "F13";
    case SDL_SCANCODE_F14: return "F14";
    case SDL_SCANCODE_F15: return "F15";
    case SDL_SCANCODE_F16: return "F16";
    case SDL_SCANCODE_F17: return "F17";
    case SDL_SCANCODE_F18: return "F18";
    case SDL_SCANCODE_F19: return "F19";
    case SDL_SCANCODE_F20: return "F20";
    case SDL_SCANCODE_F21: return "F21";
    case SDL_SCANCODE_F22: return "F22";
    case SDL_SCANCODE_F23: return "F23";
    case SDL_SCANCODE_F24: return "F24";
    default: return "";
  }
  // clang-format on
}

std::string_view Key::name() const {
  // clang-format off
  switch (code) {
    case SDL_SCANCODE_1: return "Digit1";
    case SDL_SCANCODE_2: return "Digit2";
    case SDL_SCANCODE_3: return "Digit3";
    case SDL_SCANCODE_4: return "Digit4";
    case SDL_SCANCODE_5: return "Digit5";
    case SDL_SCANCODE_6: return "Digit6";
    case SDL_SCANCODE_7: return "Digit7";
    case SDL_SCANCODE_8: return "Digit8";
    case SDL_SCANCODE_9: return "Digit9";
    case SDL_SCANCODE_0: return "Digit0";
    case SDL_SCANCODE_MINUS: return "Minus";
    case SDL_SCANCODE_EQUALS: return "Equal";

    case SDL_SCANCODE_KP_0: return "Numpad0";
    case SDL_SCANCODE_KP_1: return "Numpad1";
    case SDL_SCANCODE_KP_2: return "Numpad2";
    case SDL_SCANCODE_KP_3: return "Numpad3";
    case SDL_SCANCODE_KP_4: return "Numpad4";
    case SDL_SCANCODE_KP_5: return "Numpad5";
    case SDL_SCANCODE_KP_6: return "Numpad6";
    case SDL_SCANCODE_KP_7: return "Numpad7";
    case SDL_SCANCODE_KP_8: return "Numpad8";
    case SDL_SCANCODE_KP_9: return "Numpad9";
    case SDL_SCANCODE_KP_PERIOD: return "NumpadDecimal";
    case SDL_SCANCODE_KP_ENTER: return "NumpadEnter";
    case SDL_SCANCODE_KP_PLUS: return "NumpadAdd";
    case SDL_SCANCODE_KP_MINUS: return "NumpadSubtract";
    case SDL_SCANCODE_KP_MULTIPLY: return "NumpadMultiply";
    case SDL_SCANCODE_KP_DIVIDE: return "NumpadDivide";

    case SDL_SCANCODE_ESCAPE: return "Escape";
    case SDL_SCANCODE_INSERT: return "Insert";
    case SDL_SCANCODE_DELETE: return "Delete";
    case SDL_SCANCODE_HOME: return "Home";
    case SDL_SCANCODE_END: return "End";
    case SDL_SCANCODE_PAGEUP: return "PageUp";
    case SDL_SCANCODE_PAGEDOWN: return "PageDown";
    case SDL_SCANCODE_TAB: return "Tab";
    case SDL_SCANCODE_RETURN: return "Enter";
    case SDL_SCANCODE_BACKSPACE: return "Backspace";
  
    case SDL_SCANCODE_LCTRL: return "ControlLeft";
    case SDL_SCANCODE_RCTRL: return "ControlRight";
    case SDL_SCANCODE_LALT: return "AltLeft";
    case SDL_SCANCODE_RALT: return "AltRight";
    case SDL_SCANCODE_LGUI: return "MetaLeft";
    case SDL_SCANCODE_RGUI: return "MetaRight";
    case SDL_SCANCODE_LSHIFT: return "ShiftLeft";
    case SDL_SCANCODE_RSHIFT: return "ShiftRight";
  
    case SDL_SCANCODE_UP: return "ArrowUp";
    case SDL_SCANCODE_DOWN: return "ArrowDown";
    case SDL_SCANCODE_LEFT: return "ArrowLeft";
    case SDL_SCANCODE_RIGHT: return "ArrowRight";

    case SDL_SCANCODE_GRAVE: return "Backquote";
    case SDL_SCANCODE_LEFTBRACKET: return "BracketLeft";
    case SDL_SCANCODE_RIGHTBRACKET: return "BracketRight";
    case SDL_SCANCODE_SEMICOLON: return "Semicolon";
    case SDL_SCANCODE_APOSTROPHE: return "Quote";
    case SDL_SCANCODE_BACKSLASH: return "Backslash";
    case SDL_SCANCODE_COMMA: return "Comma";
    case SDL_SCANCODE_PERIOD: return "Period";
    case SDL_SCANCODE_SLASH: return "Slash";
    case SDL_SCANCODE_NONUSBACKSLASH: return "IntlBackslash";
    case SDL_SCANCODE_SPACE: return "Space";

    case SDL_SCANCODE_A: return "KeyA";
    case SDL_SCANCODE_B: return "KeyB";
    case SDL_SCANCODE_C: return "KeyC";
    case SDL_SCANCODE_D: return "KeyD";
    case SDL_SCANCODE_E: return "KeyE";
    case SDL_SCANCODE_F: return "KeyF";
    case SDL_SCANCODE_G: return "KeyG";
    case SDL_SCANCODE_H: return "KeyH";
    case SDL_SCANCODE_I: return "KeyI";
    case SDL_SCANCODE_J: return "KeyJ";
    case SDL_SCANCODE_K: return "KeyK";
    case SDL_SCANCODE_L: return "KeyL";
    case SDL_SCANCODE_M: return "KeyM";
    case SDL_SCANCODE_N: return "KeyN";
    case SDL_SCANCODE_O: return "KeyO";
    case SDL_SCANCODE_P: return "KeyP";
    case SDL_SCANCODE_Q: return "KeyQ";
    case SDL_SCANCODE_R: return "KeyR";
    case SDL_SCANCODE_S: return "KeyS";
    case SDL_SCANCODE_T: return "KeyT";
    case SDL_SCANCODE_U: return "KeyU";
    case SDL_SCANCODE_V: return "KeyV";
    case SDL_SCANCODE_W: return "KeyW";
    case SDL_SCANCODE_X: return "KeyX";
    case SDL_SCANCODE_Y: return "KeyY";
    case SDL_SCANCODE_Z: return "KeyZ";

    case SDL_SCANCODE_F1: return "F1";
    case SDL_SCANCODE_F2: return "F2";
    case SDL_SCANCODE_F3: return "F3";
    case SDL_SCANCODE_F4: return "F4";
    case SDL_SCANCODE_F5: return "F5";
    case SDL_SCANCODE_F6: return "F6";
    case SDL_SCANCODE_F7: return "F7";
    case SDL_SCANCODE_F8: return "F8";
    case SDL_SCANCODE_F9: return "F9";
    case SDL_SCANCODE_F10: return "F10";
    case SDL_SCANCODE_F11: return "F11";
    case SDL_SCANCODE_F12: return "F12";
    case SDL_SCANCODE_F13: return "F13";
    case SDL_SCANCODE_F14: return "F14";
    case SDL_SCANCODE_F15: return "F15";
    case SDL_SCANCODE_F16: return "F16";
    case SDL_SCANCODE_F17: return "F17";
    case SDL_SCANCODE_F18: return "F18";
    case SDL_SCANCODE_F19: return "F19";
    case SDL_SCANCODE_F20: return "F20";
    case SDL_SCANCODE_F21: return "F21";
    case SDL_SCANCODE_F22: return "F22";
    case SDL_SCANCODE_F23: return "F23";
    case SDL_SCANCODE_F24: return "F24";
    default: return "";
  }
  // clang-format on
}

Key Key::FromName(const std::string& name) {
  return {KEY_NAME_TO_KEY.at(name)};
}

void RegisterType(sol::table* module) {
  /// Represents a key on a keyboard.
  // @classmod ovis.input.Key
  sol::usertype<Key> key_type = module->new_usertype<Key>("Key", sol::no_constructor);

  /// DIGIT_1.
  // @field[type=Key] DIGIT_1
  key_type["DIGIT_1"] = sol::property(&Key::DIGIT_1);
  /// DIGIT_2.
  // @field[type=Key] DIGIT_2
  key_type["DIGIT_2"] = sol::property(&Key::DIGIT_2);
  /// DIGIT_3.
  // @field[type=Key] DIGIT_3
  key_type["DIGIT_3"] = sol::property(&Key::DIGIT_3);
  /// DIGIT_4.
  // @field[type=Key] DIGIT_4
  key_type["DIGIT_4"] = sol::property(&Key::DIGIT_4);
  /// DIGIT_5.
  // @field[type=Key] DIGIT_5
  key_type["DIGIT_5"] = sol::property(&Key::DIGIT_5);
  /// DIGIT_6.
  // @field[type=Key] DIGIT_6
  key_type["DIGIT_6"] = sol::property(&Key::DIGIT_6);
  /// DIGIT_7.
  // @field[type=Key] DIGIT_7
  key_type["DIGIT_7"] = sol::property(&Key::DIGIT_7);
  /// DIGIT_8.
  // @field[type=Key] DIGIT_8
  key_type["DIGIT_8"] = sol::property(&Key::DIGIT_8);
  /// DIGIT_9.
  // @field[type=Key] DIGIT_9
  key_type["DIGIT_9"] = sol::property(&Key::DIGIT_9);
  /// DIGIT_0.
  // @field[type=Key] DIGIT_0
  key_type["DIGIT_0"] = sol::property(&Key::DIGIT_0);
  /// MINUS.
  // @field[type=Key] MINUS
  key_type["MINUS"] = sol::property(&Key::MINUS);
  /// EQUAL.
  // @field[type=Key] EQUAL
  key_type["EQUAL"] = sol::property(&Key::EQUAL);
  /// NUMPAD_0.
  // @field[type=Key] NUMPAD_0
  key_type["NUMPAD_0"] = sol::property(&Key::NUMPAD_0);
  /// NUMPAD_1.
  // @field[type=Key] NUMPAD_1
  key_type["NUMPAD_1"] = sol::property(&Key::NUMPAD_1);
  /// NUMPAD_2.
  // @field[type=Key] NUMPAD_2
  key_type["NUMPAD_2"] = sol::property(&Key::NUMPAD_2);
  /// NUMPAD_3.
  // @field[type=Key] NUMPAD_3
  key_type["NUMPAD_3"] = sol::property(&Key::NUMPAD_3);
  /// NUMPAD_4.
  // @field[type=Key] NUMPAD_4
  key_type["NUMPAD_4"] = sol::property(&Key::NUMPAD_4);
  /// NUMPAD_5.
  // @field[type=Key] NUMPAD_5
  key_type["NUMPAD_5"] = sol::property(&Key::NUMPAD_5);
  /// NUMPAD_6.
  // @field[type=Key] NUMPAD_6
  key_type["NUMPAD_6"] = sol::property(&Key::NUMPAD_6);
  /// NUMPAD_7.
  // @field[type=Key] NUMPAD_7
  key_type["NUMPAD_7"] = sol::property(&Key::NUMPAD_7);
  /// NUMPAD_8.
  // @field[type=Key] NUMPAD_8
  key_type["NUMPAD_8"] = sol::property(&Key::NUMPAD_8);
  /// NUMPAD_9.
  // @field[type=Key] NUMPAD_9
  key_type["NUMPAD_9"] = sol::property(&Key::NUMPAD_9);
  /// NUMPAD_DECIMAL.
  // @field[type=Key] NUMPAD_DECIMAL
  key_type["NUMPAD_DECIMAL"] = sol::property(&Key::NUMPAD_DECIMAL);
  /// NUMPAD_ENTER.
  // @field[type=Key] NUMPAD_ENTER
  key_type["NUMPAD_ENTER"] = sol::property(&Key::NUMPAD_ENTER);
  /// NUMPAD_ADD.
  // @field[type=Key] NUMPAD_ADD
  key_type["NUMPAD_ADD"] = sol::property(&Key::NUMPAD_ADD);
  /// NUMPAD_SUBTRACT.
  // @field[type=Key] NUMPAD_SUBTRACT
  key_type["NUMPAD_SUBTRACT"] = sol::property(&Key::NUMPAD_SUBTRACT);
  /// NUMPAD_MULTIPLY.
  // @field[type=Key] NUMPAD_MULTIPLY
  key_type["NUMPAD_MULTIPLY"] = sol::property(&Key::NUMPAD_MULTIPLY);
  /// NUMPAD_DIVIDE.
  // @field[type=Key] NUMPAD_DIVIDE
  key_type["NUMPAD_DIVIDE"] = sol::property(&Key::NUMPAD_DIVIDE);
  /// ESCAPE.
  // @field[type=Key] ESCAPE
  key_type["ESCAPE"] = sol::property(&Key::ESCAPE);
  /// INSERT.
  // @field[type=Key] INSERT
  key_type["INSERT"] = sol::property(&Key::INSERT);
  /// DELETE.
  // @field[type=Key] DELETE
  key_type["DELETE"] = sol::property(&Key::DELETE);
  /// HOME.
  // @field[type=Key] HOME
  key_type["HOME"] = sol::property(&Key::HOME);
  /// END.
  // @field[type=Key] END
  key_type["END"] = sol::property(&Key::END);
  /// PAGE_UP.
  // @field[type=Key] PAGE_UP
  key_type["PAGE_UP"] = sol::property(&Key::PAGE_UP);
  /// PAGE_DOWN.
  // @field[type=Key] PAGE_DOWN
  key_type["PAGE_DOWN"] = sol::property(&Key::PAGE_DOWN);
  /// TAB.
  // @field[type=Key] TAB
  key_type["TAB"] = sol::property(&Key::TAB);
  /// ENTER.
  // @field[type=Key] ENTER
  key_type["ENTER"] = sol::property(&Key::ENTER);
  /// BACKSPACE.
  // @field[type=Key] BACKSPACE
  key_type["BACKSPACE"] = sol::property(&Key::BACKSPACE);
  /// CONTROL_LEFT.
  // @field[type=Key] CONTROL_LEFT
  key_type["CONTROL_LEFT"] = sol::property(&Key::CONTROL_LEFT);
  /// CONTROL_RIGHT.
  // @field[type=Key] CONTROL_RIGHT
  key_type["CONTROL_RIGHT"] = sol::property(&Key::CONTROL_RIGHT);
  /// ALT_LEFT.
  // @field[type=Key] ALT_LEFT
  key_type["ALT_LEFT"] = sol::property(&Key::ALT_LEFT);
  /// ALT_RIGHT.
  // @field[type=Key] ALT_RIGHT
  key_type["ALT_RIGHT"] = sol::property(&Key::ALT_RIGHT);
  /// META_LEFT.
  // @field[type=Key] META_LEFT
  key_type["META_LEFT"] = sol::property(&Key::META_LEFT);
  /// META_RIGHT.
  // @field[type=Key] META_RIGHT
  key_type["META_RIGHT"] = sol::property(&Key::META_RIGHT);
  /// SHIFT_LEFT.
  // @field[type=Key] SHIFT_LEFT
  key_type["SHIFT_LEFT"] = sol::property(&Key::SHIFT_LEFT);
  /// SHIFT_RIGHT.
  // @field[type=Key] SHIFT_RIGHT
  key_type["SHIFT_RIGHT"] = sol::property(&Key::SHIFT_RIGHT);
  /// ARROW_UP.
  // @field[type=Key] ARROW_UP
  key_type["ARROW_UP"] = sol::property(&Key::ARROW_UP);
  /// ARROW_DOWN.
  // @field[type=Key] ARROW_DOWN
  key_type["ARROW_DOWN"] = sol::property(&Key::ARROW_DOWN);
  /// ARROW_LEFT.
  // @field[type=Key] ARROW_LEFT
  key_type["ARROW_LEFT"] = sol::property(&Key::ARROW_LEFT);
  /// ARROW_RIGHT.
  // @field[type=Key] ARROW_RIGHT
  key_type["ARROW_RIGHT"] = sol::property(&Key::ARROW_RIGHT);
  /// BACKQUOTE.
  // @field[type=Key] BACKQUOTE
  key_type["BACKQUOTE"] = sol::property(&Key::BACKQUOTE);
  /// BRACKET_LEFT.
  // @field[type=Key] BRACKET_LEFT
  key_type["BRACKET_LEFT"] = sol::property(&Key::BRACKET_LEFT);
  /// BRACKET_RIGHT.
  // @field[type=Key] BRACKET_RIGHT
  key_type["BRACKET_RIGHT"] = sol::property(&Key::BRACKET_RIGHT);
  /// SEMICOLON.
  // @field[type=Key] SEMICOLON
  key_type["SEMICOLON"] = sol::property(&Key::SEMICOLON);
  /// QUOTE.
  // @field[type=Key] QUOTE
  key_type["QUOTE"] = sol::property(&Key::QUOTE);
  /// BACKSLASH.
  // @field[type=Key] BACKSLASH
  key_type["BACKSLASH"] = sol::property(&Key::BACKSLASH);
  /// COMMA.
  // @field[type=Key] COMMA
  key_type["COMMA"] = sol::property(&Key::COMMA);
  /// PERIOD.
  // @field[type=Key] PERIOD
  key_type["PERIOD"] = sol::property(&Key::PERIOD);
  /// SLASH.
  // @field[type=Key] SLASH
  key_type["SLASH"] = sol::property(&Key::SLASH);
  /// INTL_BACKSLASH.
  // @field[type=Key] INTL_BACKSLASH
  key_type["INTL_BACKSLASH"] = sol::property(&Key::INTL_BACKSLASH);
  /// SPACE.
  // @field[type=Key] SPACE
  key_type["SPACE"] = sol::property(&Key::SPACE);
  /// KEY_A.
  // @field[type=Key] KEY_A
  key_type["KEY_A"] = sol::property(&Key::KEY_A);
  /// KEY_B.
  // @field[type=Key] KEY_B
  key_type["KEY_B"] = sol::property(&Key::KEY_B);
  /// KEY_C.
  // @field[type=Key] KEY_C
  key_type["KEY_C"] = sol::property(&Key::KEY_C);
  /// KEY_D.
  // @field[type=Key] KEY_D
  key_type["KEY_D"] = sol::property(&Key::KEY_D);
  /// KEY_E.
  // @field[type=Key] KEY_E
  key_type["KEY_E"] = sol::property(&Key::KEY_E);
  /// KEY_F.
  // @field[type=Key] KEY_F
  key_type["KEY_F"] = sol::property(&Key::KEY_F);
  /// KEY_G.
  // @field[type=Key] KEY_G
  key_type["KEY_G"] = sol::property(&Key::KEY_G);
  /// KEY_H.
  // @field[type=Key] KEY_H
  key_type["KEY_H"] = sol::property(&Key::KEY_H);
  /// KEY_I.
  // @field[type=Key] KEY_I
  key_type["KEY_I"] = sol::property(&Key::KEY_I);
  /// KEY_J.
  // @field[type=Key] KEY_J
  key_type["KEY_J"] = sol::property(&Key::KEY_J);
  /// KEY_K.
  // @field[type=Key] KEY_K
  key_type["KEY_K"] = sol::property(&Key::KEY_K);
  /// KEY_L.
  // @field[type=Key] KEY_L
  key_type["KEY_L"] = sol::property(&Key::KEY_L);
  /// KEY_M.
  // @field[type=Key] KEY_M
  key_type["KEY_M"] = sol::property(&Key::KEY_M);
  /// KEY_N.
  // @field[type=Key] KEY_N
  key_type["KEY_N"] = sol::property(&Key::KEY_N);
  /// KEY_O.
  // @field[type=Key] KEY_O
  key_type["KEY_O"] = sol::property(&Key::KEY_O);
  /// KEY_P.
  // @field[type=Key] KEY_P
  key_type["KEY_P"] = sol::property(&Key::KEY_P);
  /// KEY_Q.
  // @field[type=Key] KEY_Q
  key_type["KEY_Q"] = sol::property(&Key::KEY_Q);
  /// KEY_R.
  // @field[type=Key] KEY_R
  key_type["KEY_R"] = sol::property(&Key::KEY_R);
  /// KEY_S.
  // @field[type=Key] KEY_S
  key_type["KEY_S"] = sol::property(&Key::KEY_S);
  /// KEY_T.
  // @field[type=Key] KEY_T
  key_type["KEY_T"] = sol::property(&Key::KEY_T);
  /// KEY_U.
  // @field[type=Key] KEY_U
  key_type["KEY_U"] = sol::property(&Key::KEY_U);
  /// KEY_V.
  // @field[type=Key] KEY_V
  key_type["KEY_V"] = sol::property(&Key::KEY_V);
  /// KEY_W.
  // @field[type=Key] KEY_W
  key_type["KEY_W"] = sol::property(&Key::KEY_W);
  /// KEY_X.
  // @field[type=Key] KEY_X
  key_type["KEY_X"] = sol::property(&Key::KEY_X);
  /// KEY_Y.
  // @field[type=Key] KEY_Y
  key_type["KEY_Y"] = sol::property(&Key::KEY_Y);
  /// KEY_Z.
  // @field[type=Key] KEY_Z
  key_type["KEY_Z"] = sol::property(&Key::KEY_Z);
  /// F1.
  // @field[type=Key] F1
  key_type["F1"] = sol::property(&Key::F1);
  /// F2.
  // @field[type=Key] F2
  key_type["F2"] = sol::property(&Key::F2);
  /// F3.
  // @field[type=Key] F3
  key_type["F3"] = sol::property(&Key::F3);
  /// F4.
  // @field[type=Key] F4
  key_type["F4"] = sol::property(&Key::F4);
  /// F5.
  // @field[type=Key] F5
  key_type["F5"] = sol::property(&Key::F5);
  /// F6.
  // @field[type=Key] F6
  key_type["F6"] = sol::property(&Key::F6);
  /// F7.
  // @field[type=Key] F7
  key_type["F7"] = sol::property(&Key::F7);
  /// F8.
  // @field[type=Key] F8
  key_type["F8"] = sol::property(&Key::F8);
  /// F9.
  // @field[type=Key] F9
  key_type["F9"] = sol::property(&Key::F9);
  /// F10.
  // @field[type=Key] F10
  key_type["F10"] = sol::property(&Key::F10);
  /// F11.
  // @field[type=Key] F11
  key_type["F11"] = sol::property(&Key::F11);
  /// F12.
  // @field[type=Key] F12
  key_type["F12"] = sol::property(&Key::F12);
  /// F13.
  // @field[type=Key] F13
  key_type["F13"] = sol::property(&Key::F13);
  /// F14.
  // @field[type=Key] F14
  key_type["F14"] = sol::property(&Key::F14);
  /// F15.
  // @field[type=Key] F15
  key_type["F15"] = sol::property(&Key::F15);
  /// F16.
  // @field[type=Key] F16
  key_type["F16"] = sol::property(&Key::F16);
  /// F17.
  // @field[type=Key] F17
  key_type["F17"] = sol::property(&Key::F17);
  /// F18.
  // @field[type=Key] F18
  key_type["F18"] = sol::property(&Key::F18);
  /// F19.
  // @field[type=Key] F19
  key_type["F19"] = sol::property(&Key::F19);
  /// F20.
  // @field[type=Key] F20
  key_type["F20"] = sol::property(&Key::F20);
  /// F21.
  // @field[type=Key] F21
  key_type["F21"] = sol::property(&Key::F21);
  /// F22.
  // @field[type=Key] F22
  key_type["F22"] = sol::property(&Key::F22);
  /// F23.
  // @field[type=Key] F23
  key_type["F23"] = sol::property(&Key::F23);
  /// F24.
  // @field[type=Key] F24
  key_type["F24"] = sol::property(&Key::F24);
}

}  // namespace ovis
