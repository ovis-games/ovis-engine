#pragma once

#include <cstdint>
#include <limits>
#include <string_view>

#include <SDL2/SDL_scancode.h>
#include "ovis/core/vm_bindings.hpp"

namespace ovis {

struct MouseButton {
  uint8_t code;

  std::string_view id() const;
  std::string_view name() const;

  static MouseButton FromName(std::string_view name);

  static constexpr MouseButton Left() { return {0}; }
  static constexpr MouseButton Middle() { return {1}; }
  static constexpr MouseButton Right() { return {2}; }
  static constexpr MouseButton Four() { return {3}; }
  static constexpr MouseButton Five() { return {4}; }

  OVIS_VM_DECLARE_TYPE_BINDING();
};
static_assert(sizeof(MouseButton) == 1);

inline bool operator==(MouseButton lhs, MouseButton rhs) {
  return lhs.code == rhs.code;
}

inline bool operator!=(MouseButton lhs, MouseButton rhs) {
  return lhs.code != rhs.code;
}

bool GetMouseButtonState(MouseButton button);
void SetMouseButtonState(MouseButton button, bool pressed);

}  // namespace ovis
