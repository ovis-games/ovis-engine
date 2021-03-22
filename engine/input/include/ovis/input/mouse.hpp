#pragma once

#include <string>
#include <unordered_map>

#include <SDL_mouse.h>
#include <sol/sol.hpp>

#include <ovis/core/event.hpp>
#include <ovis/input/keyboard.hpp>
#include <ovis/engine/viewport.hpp>

namespace ovis {
class Input {
 public:
  Input();

  inline void SetKeyState(SDL_Scancode scan_code, bool pressed) { key_states_[scan_code] = pressed; }
  bool GetKeyState(Key key) const { return key_states_[key.code]; }

  inline void SetMouseButtonState(MouseButton button, bool pressed) {
    mouse_button_states_[static_cast<uint8_t>(button) - 1] = pressed;
  }

  inline bool GetMouseButtonState(MouseButton button) { return mouse_button_states_[static_cast<uint8_t>(button) - 1]; }

  static void RegisterType(sol::table* module);

 private:
  bool key_states_[SDL_NUM_SCANCODES] = {false};
  bool mouse_button_states_[5] = {false};
};

Input* input();

class KeyboardEvent : public Event {
 public:
  inline KeyboardEvent(std::string type, Key key) : Event(std::move(type)), key_(key) {}

  inline Key key() const { return key_; }

 private:
  Key key_;
};

class KeyPressEvent : public KeyboardEvent {
 public:
  inline static const std::string TYPE = "KeyPress";

  inline KeyPressEvent(Key key) : KeyboardEvent(TYPE, key) {}
};

class KeyReleaseEvent : public KeyboardEvent {
 public:
  inline static const std::string TYPE = "KeyRelease";

  inline KeyReleaseEvent(Key key) : KeyboardEvent(TYPE, key) {}
};

class TextInputEvent : public Event {
 public:
  inline static const std::string TYPE = "TextInput";

  inline TextInputEvent(std::string text) : Event(TYPE), text_(text) {}

  const std::string& text() const { return text_; }

 private:
  std::string text_;
};

class MouseEvent : public Event {
 public:
  inline MouseEvent(std::string type, Viewport* viewport, Vector2 device_coordinates)
      : Event(std::move(type)), viewport_(viewport), device_coordinates_(device_coordinates) {}

  inline Viewport* viewport() const { return viewport_; }
  inline Vector2 device_coordinates() const { return device_coordinates_; }
  inline Vector2 normalized_device_coordinates() const {
    SDL_assert(viewport() != nullptr);
    return viewport_->DeviceCoordinatesToNormalizedDeviceCoordinates(device_coordinates_);
  }

 private:
  Viewport* viewport_;
  Vector2 device_coordinates_;
};

class MouseMoveEvent : public MouseEvent {
 public:
  inline static const std::string TYPE = "MouseMove";

  inline MouseMoveEvent(Viewport* viewport, Vector2 device_coordinates, Vector2 relative_device_coordinates)
      : MouseEvent(TYPE, viewport, device_coordinates), relative_device_coordinates_(relative_device_coordinates) {}

  inline Vector2 relative_device_coordinates() const { return relative_device_coordinates_; }
  inline Vector2 relative_normalized_device_coordinates() const {
    SDL_assert(viewport() != nullptr);
    return viewport()->DeviceCoordinatesToNormalizedDeviceCoordinates(relative_device_coordinates_);
  }

 private:
  Vector2 relative_device_coordinates_;
};

class MouseButtonEvent : public MouseEvent {
 public:
  inline MouseButtonEvent(std::string type, Viewport* viewport, Vector2 device_coordinates, MouseButton button)
      : MouseEvent(std::move(type), viewport, device_coordinates), button_(button) {}

  inline MouseButton button() const { return button_; }

 private:
  MouseButton button_;
};

class MouseButtonPressEvent : public MouseButtonEvent {
 public:
  inline static const std::string TYPE = "MouseButtonPress";

  inline MouseButtonPressEvent(Viewport* viewport, Vector2 device_coordinates, MouseButton button)
      : MouseButtonEvent(TYPE, viewport, device_coordinates, button) {}
};

class MouseButtonReleaseEvent : public MouseButtonEvent {
 public:
  inline static const std::string TYPE = "MouseButtonRelease";

  inline MouseButtonReleaseEvent(Viewport* viewport, Vector2 device_coordinates, MouseButton button)
      : MouseButtonEvent(TYPE, viewport, device_coordinates, button) {}
};

class MouseWheelEvent : public Event {
 public:
  inline static const std::string TYPE = "MouseWheelEvent";

  inline MouseWheelEvent(Vector2 delta) : Event(TYPE), delta_(delta) {}

  inline Vector2 delta() const { return delta_; }

 private:
  Vector2 delta_;
};

}  // namespace ovis
