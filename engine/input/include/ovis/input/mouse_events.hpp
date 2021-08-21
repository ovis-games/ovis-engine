#pragma once

#include <sol/sol.hpp>

#include <ovis/core/event.hpp>
#include <ovis/core/scene_viewport.hpp>
#include <ovis/core/vector.hpp>
#include <ovis/input/mouse_button.hpp>

namespace ovis {

class MouseEvent : public Event {
 public:
  inline MouseEvent(std::string type, SceneViewport* viewport, Vector2 screen_space_position)
      : Event(std::move(type)), viewport_(viewport), screen_space_position_(screen_space_position) {}

  inline SceneViewport* viewport() const { return viewport_; }
  inline Vector2 screen_space_position() const { return screen_space_position_; }

 private:
  SceneViewport* viewport_;
  Vector2 screen_space_position_;
};

class MouseMoveEvent : public MouseEvent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  inline static const std::string TYPE = "MouseMove";

  inline MouseMoveEvent(SceneViewport* viewport, Vector2 screen_space_position, Vector2 relative_screen_space_position)
      : MouseEvent(TYPE, viewport, screen_space_position),
        relative_screen_space_position_(relative_screen_space_position) {}

  inline Vector2 relative_screen_space_position() const { return relative_screen_space_position_; }

  static void RegisterType(sol::table* module);

 private:
  Vector2 relative_screen_space_position_;
};

class MouseButtonEvent : public MouseEvent {
 public:
  inline MouseButtonEvent(std::string type, SceneViewport* viewport, Vector2 screen_space_position, MouseButton button)
      : MouseEvent(std::move(type), viewport, screen_space_position), button_(button) {}

  inline MouseButton button() const { return button_; }

  static void RegisterType(sol::table* module);

 private:
  MouseButton button_;
};

class MouseButtonPressEvent : public MouseButtonEvent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  inline static const std::string TYPE = "MouseButtonPress";

  inline MouseButtonPressEvent(SceneViewport* viewport, Vector2 screen_space_position, MouseButton button)
      : MouseButtonEvent(TYPE, viewport, screen_space_position, button) {}

  static void RegisterType(sol::table* module);
};

class MouseButtonReleaseEvent : public MouseButtonEvent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  inline static const std::string TYPE = "MouseButtonRelease";

  inline MouseButtonReleaseEvent(SceneViewport* viewport, Vector2 screen_space_position, MouseButton button)
      : MouseButtonEvent(TYPE, viewport, screen_space_position, button) {}

  static void RegisterType(sol::table* module);
};

enum class MouseWheelDeltaMode {
  PIXELS = 0,
  LINES = 1,
  PAGES = 2,
  UNKNDOWN = 3,
};

class MouseWheelEvent : public Event {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  inline static const std::string TYPE = "MouseWheelEvent";

  inline MouseWheelEvent(Vector2 delta, MouseWheelDeltaMode mode = MouseWheelDeltaMode::UNKNDOWN)
      : Event(TYPE), delta_(delta), mode_(mode) {}

  inline Vector2 delta() const { return delta_; }
  inline MouseWheelDeltaMode mode() const { return mode_; }

  static void RegisterType(sol::table* module);

 private:
  Vector2 delta_;
  MouseWheelDeltaMode mode_;
};

}  // namespace ovis
