#pragma once

#include <string>
#include <unordered_map>

#include <SDL_mouse.h>

#include <ovis/engine/event.hpp>
#include <ovis/engine/key.hpp>
#include <ovis/engine/viewport.hpp>

namespace ovis {

enum class MouseButton : uint8_t {
  LEFT = SDL_BUTTON_LEFT,
  MIDDLE = SDL_BUTTON_MIDDLE,
  RIGHT = SDL_BUTTON_RIGHT,
  EXTRA1 = SDL_BUTTON_X1,
  EXTRA2 = SDL_BUTTON_X2,
};

class Input {
 public:
  Input();

  inline void SetKeyState(SDL_Scancode scan_code, bool pressed) { key_states_[scan_code] = pressed; }
  bool GetKeyState(Key key) const { return key_states_[key.code]; }

  inline void SetMouseButtonState(MouseButton button, bool pressed) {
    mouse_button_states_[static_cast<uint8_t>(button) - 1] = pressed;
  }

  inline bool GetMouseButtonState(MouseButton button) {
    return mouse_button_states_[static_cast<uint8_t>(button) - 1];
  }

  static void RegisterToLua();

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
  inline MouseEvent(std::string type, Viewport* viewport, vector2 device_coordinates)
      : Event(std::move(type)), viewport_(viewport), device_coordinates_(device_coordinates) {}

  inline Viewport* viewport() const { return viewport_; }
  inline vector2 device_coordinates() const { return device_coordinates_; }
  inline vector2 normalized_device_coordinates() const {
    SDL_assert(viewport() != nullptr);
    return viewport_->DeviceCoordinatesToNormalizedDeviceCoordinates(device_coordinates_);
  }

 private:
  Viewport* viewport_;
  vector2 device_coordinates_;
};

class MouseMoveEvent : public MouseEvent {
 public:
  inline static const std::string TYPE = "MouseMove";

  inline MouseMoveEvent(Viewport* viewport, vector2 device_coordinates, vector2 relative_device_coordinates)
      : MouseEvent(TYPE, viewport, device_coordinates), relative_device_coordinates_(relative_device_coordinates) {}

  inline vector2 relative_device_coordinates() const { return relative_device_coordinates_; }
  inline vector2 relative_normalized_device_coordinates() const {
    SDL_assert(viewport() != nullptr);
    return viewport()->DeviceCoordinatesToNormalizedDeviceCoordinates(relative_device_coordinates_);
  }

 private:
  vector2 relative_device_coordinates_;
};

class MouseButtonEvent : public MouseEvent {
 public:
  inline MouseButtonEvent(std::string type, Viewport* viewport, vector2 device_coordinates, MouseButton button)
      : MouseEvent(std::move(type), viewport, device_coordinates), button_(button) {}

  inline MouseButton button() const { return button_; }

 private:
  MouseButton button_;
};

class MouseButtonPressEvent : public MouseButtonEvent {
 public:
  inline static const std::string TYPE = "MouseButtonPress";

  inline MouseButtonPressEvent(Viewport* viewport, vector2 device_coordinates, MouseButton button)
      : MouseButtonEvent(TYPE, viewport, device_coordinates, button) {}
};

class MouseButtonReleaseEvent : public MouseButtonEvent {
 public:
  inline static const std::string TYPE = "MouseButtonRelease";

  inline MouseButtonReleaseEvent(Viewport* viewport, vector2 device_coordinates, MouseButton button)
      : MouseButtonEvent(TYPE, viewport, device_coordinates, button) {}
};

class MouseWheelEvent : public Event {
 public:
  inline static const std::string TYPE = "MouseWheelEvent";

  inline MouseWheelEvent(int delta_x, int delta_y) : Event(TYPE), delta_x_(delta_x), delta_y_(delta_y) {}

  inline int delta_x() const { return delta_x_; }
  inline int delta_y() const { return delta_y_; }

 private:
  int delta_x_;
  int delta_y_;
};

}  // namespace ovis
