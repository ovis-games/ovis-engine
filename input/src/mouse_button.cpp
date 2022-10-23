#include "ovis/input/mouse_button.hpp"

#include <unordered_map>

#include "ovis/core/main_vm.hpp"

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
    default: assert(false);  return "";
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
    default: assert(false);  return "";
  }
  // clang-format on
}

MouseButton MouseButton::FromName(std::string_view name) {
  return {BUTTON_NAME_TO_BUTTON.at(name)};
}

OVIS_VM_DEFINE_TYPE_BINDING(Input, MouseButton) {
  MouseButton_type->constants.insert(std::make_pair("Left", main_vm->CreateValue(MouseButton::Left())));
  MouseButton_type->constants.insert(std::make_pair("Right", main_vm->CreateValue(MouseButton::Right())));
  MouseButton_type->constants.insert(std::make_pair("Middle", main_vm->CreateValue(MouseButton::Middle())));
  MouseButton_type->constants.insert(std::make_pair("Four", main_vm->CreateValue(MouseButton::Four())));
  MouseButton_type->constants.insert(std::make_pair("Five", main_vm->CreateValue(MouseButton::Five())));
}

namespace {
bool mouse_button_states[5] = {false};
}

bool GetMouseButtonState(MouseButton button) {
  assert(button.code < 5);
  return mouse_button_states[button.code];
}

void SetMouseButtonState(MouseButton button, bool pressed) {
  assert(button.code < 5);
  mouse_button_states[button.code] = pressed;
}

}  // namespace ovis
