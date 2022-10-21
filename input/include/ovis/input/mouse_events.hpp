#pragma once

#include "ovis/core/scene_viewport.hpp"
#include "ovis/core/vector.hpp"
#include "ovis/input/mouse_button.hpp"

namespace ovis {

// TODO: figure out from which viewport the event comes from

struct MouseMoveEvent {
  Vector2 screen_space_position;
  Vector2 relative_screen_space_position;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

struct MouseButtonPressEvent {
  Vector2 screen_space_position;
  MouseButton button;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

struct MouseButtonReleaseEvent {
  Vector2 screen_space_position;
  MouseButton button;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

enum class MouseWheelDeltaMode {
  PIXELS = 0,
  LINES = 1,
  PAGES = 2,
  UNKNDOWN = 3,
};

struct MouseWheelEvent {
  Vector2 delta;
  MouseWheelDeltaMode mode;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

}  // namespace ovis
