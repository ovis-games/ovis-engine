#include "ovis/input/key.hpp"

#include <unordered_map>
#include "ovis/core/main_vm.hpp"

namespace ovis {

namespace {
static const std::unordered_map<std::string_view, Key::KeyCode> KEY_NAME_TO_KEY = {
    {"1", SDL_SCANCODE_1},
    {"2", SDL_SCANCODE_2},
    {"3", SDL_SCANCODE_3},
    {"4", SDL_SCANCODE_4},
    {"5", SDL_SCANCODE_5},
    {"6", SDL_SCANCODE_6},
    {"7", SDL_SCANCODE_7},
    {"8", SDL_SCANCODE_8},
    {"9", SDL_SCANCODE_9},
    {"0", SDL_SCANCODE_0},
    {"-", SDL_SCANCODE_MINUS},
    {"=", SDL_SCANCODE_EQUALS},
    {"Numpad 0", SDL_SCANCODE_KP_0},
    {"Numpad 1", SDL_SCANCODE_KP_1},
    {"Numpad 2", SDL_SCANCODE_KP_2},
    {"Numpad 3", SDL_SCANCODE_KP_3},
    {"Numpad 4", SDL_SCANCODE_KP_4},
    {"Numpad 5", SDL_SCANCODE_KP_5},
    {"Numpad 6", SDL_SCANCODE_KP_6},
    {"Numpad 7", SDL_SCANCODE_KP_7},
    {"Numpad 8", SDL_SCANCODE_KP_8},
    {"Numpad 9", SDL_SCANCODE_KP_9},
    {"Numpad .", SDL_SCANCODE_KP_PERIOD},
    {"Numpad Enter", SDL_SCANCODE_KP_ENTER},
    {"Numpad +", SDL_SCANCODE_KP_PLUS},
    {"Numpad -", SDL_SCANCODE_KP_MINUS},
    {"Numpad *", SDL_SCANCODE_KP_MULTIPLY},
    {"Numpad /", SDL_SCANCODE_KP_DIVIDE},
    {"Escape", SDL_SCANCODE_ESCAPE},
    {"Insert", SDL_SCANCODE_INSERT},
    {"Delete", SDL_SCANCODE_DELETE},
    {"Home", SDL_SCANCODE_HOME},
    {"End", SDL_SCANCODE_END},
    {"Page Up", SDL_SCANCODE_PAGEUP},
    {"Page Down", SDL_SCANCODE_PAGEDOWN},
    {"Tab", SDL_SCANCODE_TAB},
    {"Enter", SDL_SCANCODE_RETURN},
    {"Backspace", SDL_SCANCODE_BACKSPACE},
    {"Left Control", SDL_SCANCODE_LCTRL},
    {"Right Control", SDL_SCANCODE_RCTRL},
    {"Left Alt", SDL_SCANCODE_LALT},
    {"Right Alt", SDL_SCANCODE_RALT},
    {"Left Meta", SDL_SCANCODE_LGUI},
    {"Right Meta", SDL_SCANCODE_RGUI},
    {"Left Shift", SDL_SCANCODE_LSHIFT},
    {"Right Shift", SDL_SCANCODE_RSHIFT},
    {"Arrow Up", SDL_SCANCODE_UP},
    {"Arrow Down", SDL_SCANCODE_DOWN},
    {"Arrow Left", SDL_SCANCODE_LEFT},
    {"Arrow Right", SDL_SCANCODE_RIGHT},
    {"`", SDL_SCANCODE_GRAVE},
    {"[", SDL_SCANCODE_LEFTBRACKET},
    {"]", SDL_SCANCODE_RIGHTBRACKET},
    {";", SDL_SCANCODE_SEMICOLON},
    {"\"", SDL_SCANCODE_APOSTROPHE},
    {"\\", SDL_SCANCODE_BACKSLASH},
    {",", SDL_SCANCODE_COMMA},
    {".", SDL_SCANCODE_PERIOD},
    {"/", SDL_SCANCODE_SLASH},
    {"IntlBackslash", SDL_SCANCODE_NONUSBACKSLASH},
    {"Space", SDL_SCANCODE_SPACE},
    {"A", SDL_SCANCODE_A},
    {"B", SDL_SCANCODE_B},
    {"C", SDL_SCANCODE_C},
    {"D", SDL_SCANCODE_D},
    {"E", SDL_SCANCODE_E},
    {"F", SDL_SCANCODE_F},
    {"G", SDL_SCANCODE_G},
    {"H", SDL_SCANCODE_H},
    {"I", SDL_SCANCODE_I},
    {"J", SDL_SCANCODE_J},
    {"K", SDL_SCANCODE_K},
    {"L", SDL_SCANCODE_L},
    {"M", SDL_SCANCODE_M},
    {"N", SDL_SCANCODE_N},
    {"O", SDL_SCANCODE_O},
    {"P", SDL_SCANCODE_P},
    {"Q", SDL_SCANCODE_Q},
    {"R", SDL_SCANCODE_R},
    {"S", SDL_SCANCODE_S},
    {"T", SDL_SCANCODE_T},
    {"U", SDL_SCANCODE_U},
    {"V", SDL_SCANCODE_V},
    {"W", SDL_SCANCODE_W},
    {"X", SDL_SCANCODE_X},
    {"Y", SDL_SCANCODE_Y},
    {"Z", SDL_SCANCODE_Z},
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
    case SDL_SCANCODE_1: return "1";
    case SDL_SCANCODE_2: return "2";
    case SDL_SCANCODE_3: return "3";
    case SDL_SCANCODE_4: return "4";
    case SDL_SCANCODE_5: return "5";
    case SDL_SCANCODE_6: return "6";
    case SDL_SCANCODE_7: return "7";
    case SDL_SCANCODE_8: return "8";
    case SDL_SCANCODE_9: return "9";
    case SDL_SCANCODE_0: return "0";
    case SDL_SCANCODE_MINUS: return "-";
    case SDL_SCANCODE_EQUALS: return "=";

    case SDL_SCANCODE_KP_0: return "Numpad 0";
    case SDL_SCANCODE_KP_1: return "Numpad 1";
    case SDL_SCANCODE_KP_2: return "Numpad 2";
    case SDL_SCANCODE_KP_3: return "Numpad 3";
    case SDL_SCANCODE_KP_4: return "Numpad 4";
    case SDL_SCANCODE_KP_5: return "Numpad 5";
    case SDL_SCANCODE_KP_6: return "Numpad 6";
    case SDL_SCANCODE_KP_7: return "Numpad 7";
    case SDL_SCANCODE_KP_8: return "Numpad 8";
    case SDL_SCANCODE_KP_9: return "Numpad 9";
    case SDL_SCANCODE_KP_PERIOD: return "Numpad Decimal";
    case SDL_SCANCODE_KP_ENTER: return "Numpad Enter";
    case SDL_SCANCODE_KP_PLUS: return "Numpad Add";
    case SDL_SCANCODE_KP_MINUS: return "Numpad Subtract";
    case SDL_SCANCODE_KP_MULTIPLY: return "Numpad Multiply";
    case SDL_SCANCODE_KP_DIVIDE: return "Numpad Divide";

    case SDL_SCANCODE_ESCAPE: return "Escape";
    case SDL_SCANCODE_INSERT: return "Insert";
    case SDL_SCANCODE_DELETE: return "Delete";
    case SDL_SCANCODE_HOME: return "Home";
    case SDL_SCANCODE_END: return "End";
    case SDL_SCANCODE_PAGEUP: return "Page Up";
    case SDL_SCANCODE_PAGEDOWN: return "Page Down";
    case SDL_SCANCODE_TAB: return "Tab";
    case SDL_SCANCODE_RETURN: return "Enter";
    case SDL_SCANCODE_BACKSPACE: return "Backspace";
  
    case SDL_SCANCODE_LCTRL: return "Left Control";
    case SDL_SCANCODE_RCTRL: return "Right Control";
    case SDL_SCANCODE_LALT: return "Left Alt";
    case SDL_SCANCODE_RALT: return "Right Alt";
    case SDL_SCANCODE_LGUI: return "Left Meta";
    case SDL_SCANCODE_RGUI: return "Right Meta";
    case SDL_SCANCODE_LSHIFT: return "Left Shift";
    case SDL_SCANCODE_RSHIFT: return "Right Shift";
  
    case SDL_SCANCODE_UP: return "Arrow Up";
    case SDL_SCANCODE_DOWN: return "Arrow Down";
    case SDL_SCANCODE_LEFT: return "Arrow Left";
    case SDL_SCANCODE_RIGHT: return "Arrow Right";

    case SDL_SCANCODE_GRAVE: return "`";
    case SDL_SCANCODE_LEFTBRACKET: return "[";
    case SDL_SCANCODE_RIGHTBRACKET: return "]";
    case SDL_SCANCODE_SEMICOLON: return ";";
    case SDL_SCANCODE_APOSTROPHE: return "\"";
    case SDL_SCANCODE_BACKSLASH: return "\\";
    case SDL_SCANCODE_COMMA: return ",";
    case SDL_SCANCODE_PERIOD: return ".";
    case SDL_SCANCODE_SLASH: return "/";
    case SDL_SCANCODE_NONUSBACKSLASH: return "IntlBackslash";
    case SDL_SCANCODE_SPACE: return "Space";

    case SDL_SCANCODE_A: return "A";
    case SDL_SCANCODE_B: return "B";
    case SDL_SCANCODE_C: return "C";
    case SDL_SCANCODE_D: return "D";
    case SDL_SCANCODE_E: return "E";
    case SDL_SCANCODE_F: return "F";
    case SDL_SCANCODE_G: return "G";
    case SDL_SCANCODE_H: return "H";
    case SDL_SCANCODE_I: return "I";
    case SDL_SCANCODE_J: return "J";
    case SDL_SCANCODE_K: return "K";
    case SDL_SCANCODE_L: return "L";
    case SDL_SCANCODE_M: return "M";
    case SDL_SCANCODE_N: return "N";
    case SDL_SCANCODE_O: return "O";
    case SDL_SCANCODE_P: return "P";
    case SDL_SCANCODE_Q: return "Q";
    case SDL_SCANCODE_R: return "R";
    case SDL_SCANCODE_S: return "S";
    case SDL_SCANCODE_T: return "T";
    case SDL_SCANCODE_U: return "U";
    case SDL_SCANCODE_V: return "V";
    case SDL_SCANCODE_W: return "W";
    case SDL_SCANCODE_X: return "X";
    case SDL_SCANCODE_Y: return "Y";
    case SDL_SCANCODE_Z: return "Z";

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
    default: assert(false); return "";
  }
  // clang-format on
}

Key Key::FromName(std::string_view name) {
  return {KEY_NAME_TO_KEY.at(name)};
}

OVIS_VM_DEFINE_TYPE_BINDING(Input, Key) {
  for (const auto& [name, key_code] : KEY_NAME_TO_KEY) {
    Key_type->constants.insert(std::make_pair(name, main_vm->CreateValue(Key{.code = key_code})));
  }
}

namespace {
std::vector<Key> MakeKeysVector() {
  std::vector<Key> keys;
  keys.reserve(KEY_NAME_TO_KEY.size());
  std::transform(KEY_NAME_TO_KEY.cbegin(), KEY_NAME_TO_KEY.cend(), std::back_inserter(keys),
                 [](std::pair<std::string_view, Key::KeyCode> key) -> Key { return {key.second}; });
  return keys;
}

static const std::vector<Key> KEYS = MakeKeysVector();
}  // namespace

const std::vector<Key>& Keys() {
  return KEYS;
}
namespace {
bool key_states[SDL_NUM_SCANCODES] = {false};
}

bool IsKeyPressed(Key key) {
  assert(key.code < SDL_NUM_SCANCODES);
  return key_states[key.code];
}

void SetKeyState(Key key, bool pressed) {
  assert(key.code < SDL_NUM_SCANCODES);
  key_states[key.code] = pressed;
}

bool IsModifierPressed(KeyModifier modifier) {
  switch (modifier) {
    case KeyModifier::SHIFT:
      return IsKeyPressed(Key::ShiftLeft()) | IsKeyPressed(Key::ShiftRight());
    case KeyModifier::ALT:
      return IsKeyPressed(Key::AltLeft()) | IsKeyPressed(Key::AltRight());
    case KeyModifier::CONTROL:
      return IsKeyPressed(Key::ControlLeft()) | IsKeyPressed(Key::ControlRight());
    case KeyModifier::META:
      return IsKeyPressed(Key::MetaLeft()) | IsKeyPressed(Key::MetaRight());
  }
}

}  // namespace ovis
