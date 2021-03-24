#include <ovis/core/scene.hpp>
#include <ovis/input/emscripten_callbacks.hpp>
#include <ovis/input/key_events.hpp>
#include <ovis/input/mouse_events.hpp>
#include <ovis/input/text_input_event.hpp>

namespace ovis {

#if OVIS_EMSCRIPTEN

namespace {
static const std::unordered_map<std::string, Key> KEY_NAME_TO_KEY = {
    {"Digit1", {SDL_SCANCODE_1}},
    {"Digit2", {SDL_SCANCODE_2}},
    {"Digit3", {SDL_SCANCODE_3}},
    {"Digit4", {SDL_SCANCODE_4}},
    {"Digit5", {SDL_SCANCODE_5}},
    {"Digit6", {SDL_SCANCODE_6}},
    {"Digit7", {SDL_SCANCODE_7}},
    {"Digit8", {SDL_SCANCODE_8}},
    {"Digit9", {SDL_SCANCODE_9}},
    {"Digit0", {SDL_SCANCODE_0}},
    {"Minus", {SDL_SCANCODE_MINUS}},
    {"Equal", {SDL_SCANCODE_EQUALS}},
    {"Numpad0", {SDL_SCANCODE_KP_0}},
    {"Numpad1", {SDL_SCANCODE_KP_1}},
    {"Numpad2", {SDL_SCANCODE_KP_2}},
    {"Numpad3", {SDL_SCANCODE_KP_3}},
    {"Numpad4", {SDL_SCANCODE_KP_4}},
    {"Numpad5", {SDL_SCANCODE_KP_5}},
    {"Numpad6", {SDL_SCANCODE_KP_6}},
    {"Numpad7", {SDL_SCANCODE_KP_7}},
    {"Numpad8", {SDL_SCANCODE_KP_8}},
    {"Numpad9", {SDL_SCANCODE_KP_9}},
    {"NumpadDecimal", {SDL_SCANCODE_KP_PERIOD}},
    {"NumpadEnter", {SDL_SCANCODE_KP_ENTER}},
    {"NumpadAdd", {SDL_SCANCODE_KP_PLUS}},
    {"NumpadSubtract", {SDL_SCANCODE_KP_MINUS}},
    {"NumpadMultiply", {SDL_SCANCODE_KP_MULTIPLY}},
    {"NumpadDivide", {SDL_SCANCODE_KP_DIVIDE}},
    {"Escape", {SDL_SCANCODE_ESCAPE}},
    {"Insert", {SDL_SCANCODE_INSERT}},
    {"Delete", {SDL_SCANCODE_DELETE}},
    {"Home", {SDL_SCANCODE_HOME}},
    {"End", {SDL_SCANCODE_END}},
    {"PageUp", {SDL_SCANCODE_PAGEUP}},
    {"PageDown", {SDL_SCANCODE_PAGEDOWN}},
    {"Tab", {SDL_SCANCODE_TAB}},
    {"Enter", {SDL_SCANCODE_RETURN}},
    {"Backspace", {SDL_SCANCODE_BACKSPACE}},
    {"ControlLeft", {SDL_SCANCODE_LCTRL}},
    {"ControlRight", {SDL_SCANCODE_RCTRL}},
    {"AltLeft", {SDL_SCANCODE_LALT}},
    {"AltRight", {SDL_SCANCODE_RALT}},
    {"MetaLeft", {SDL_SCANCODE_LGUI}},
    {"MetaRight", {SDL_SCANCODE_RGUI}},
    {"ShiftLeft", {SDL_SCANCODE_LSHIFT}},
    {"ShiftRight", {SDL_SCANCODE_RSHIFT}},
    {"ArrowUp", {SDL_SCANCODE_UP}},
    {"ArrowDown", {SDL_SCANCODE_DOWN}},
    {"ArrowLeft", {SDL_SCANCODE_LEFT}},
    {"ArrowRight", {SDL_SCANCODE_RIGHT}},
    {"Backquote", {SDL_SCANCODE_GRAVE}},
    {"BracketLeft", {SDL_SCANCODE_LEFTBRACKET}},
    {"BracketRight", {SDL_SCANCODE_RIGHTBRACKET}},
    {"Semicolon", {SDL_SCANCODE_SEMICOLON}},
    {"Quote", {SDL_SCANCODE_APOSTROPHE}},
    {"Backslash", {SDL_SCANCODE_BACKSLASH}},
    {"Comma", {SDL_SCANCODE_COMMA}},
    {"Period", {SDL_SCANCODE_PERIOD}},
    {"Slash", {SDL_SCANCODE_SLASH}},
    {"IntlBackslash", {SDL_SCANCODE_NONUSBACKSLASH}},
    {"Space", {SDL_SCANCODE_SPACE}},
    {"KeyA", {SDL_SCANCODE_A}},
    {"KeyB", {SDL_SCANCODE_B}},
    {"KeyC", {SDL_SCANCODE_C}},
    {"KeyD", {SDL_SCANCODE_D}},
    {"KeyE", {SDL_SCANCODE_E}},
    {"KeyF", {SDL_SCANCODE_F}},
    {"KeyG", {SDL_SCANCODE_G}},
    {"KeyH", {SDL_SCANCODE_H}},
    {"KeyI", {SDL_SCANCODE_I}},
    {"KeyJ", {SDL_SCANCODE_J}},
    {"KeyK", {SDL_SCANCODE_K}},
    {"KeyL", {SDL_SCANCODE_L}},
    {"KeyM", {SDL_SCANCODE_M}},
    {"KeyN", {SDL_SCANCODE_N}},
    {"KeyO", {SDL_SCANCODE_O}},
    {"KeyP", {SDL_SCANCODE_P}},
    {"KeyQ", {SDL_SCANCODE_Q}},
    {"KeyR", {SDL_SCANCODE_R}},
    {"KeyS", {SDL_SCANCODE_S}},
    {"KeyT", {SDL_SCANCODE_T}},
    {"KeyU", {SDL_SCANCODE_U}},
    {"KeyV", {SDL_SCANCODE_V}},
    {"KeyW", {SDL_SCANCODE_W}},
    {"KeyX", {SDL_SCANCODE_X}},
    {"KeyY", {SDL_SCANCODE_Y}},
    {"KeyZ", {SDL_SCANCODE_Z}},
    {"F1", {SDL_SCANCODE_F1}},
    {"F2", {SDL_SCANCODE_F2}},
    {"F3", {SDL_SCANCODE_F3}},
    {"F4", {SDL_SCANCODE_F4}},
    {"F5", {SDL_SCANCODE_F5}},
    {"F6", {SDL_SCANCODE_F6}},
    {"F7", {SDL_SCANCODE_F7}},
    {"F8", {SDL_SCANCODE_F8}},
    {"F9", {SDL_SCANCODE_F9}},
    {"F10", {SDL_SCANCODE_F10}},
    {"F11", {SDL_SCANCODE_F11}},
    {"F12", {SDL_SCANCODE_F12}},
    {"F13", {SDL_SCANCODE_F13}},
    {"F14", {SDL_SCANCODE_F14}},
    {"F15", {SDL_SCANCODE_F15}},
    {"F16", {SDL_SCANCODE_F16}},
    {"F17", {SDL_SCANCODE_F17}},
    {"F18", {SDL_SCANCODE_F18}},
    {"F19", {SDL_SCANCODE_F19}},
    {"F20", {SDL_SCANCODE_F20}},
    {"F21", {SDL_SCANCODE_F21}},
    {"F22", {SDL_SCANCODE_F22}},
    {"F23", {SDL_SCANCODE_F23}},
    {"F24", {SDL_SCANCODE_F24}},
};
}

EM_BOOL HandleWheelEvent(int event_type, const EmscriptenWheelEvent* wheel_event, void* viewport) {
  SceneViewport* scene_viewport = static_cast<SceneViewport*>(viewport);
  MouseWheelEvent mouse_wheel_event(
      {static_cast<float>(-wheel_event->deltaX), static_cast<float>(-wheel_event->deltaY)});
  scene_viewport->scene()->ProcessEvent(&mouse_wheel_event);
  return !mouse_wheel_event.is_propagating();
}

EM_BOOL HandleKeyDownEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* viewport) {
  const Key key = KEY_NAME_TO_KEY.at(keyboard_event->code);
  SetKeyState(key, true);

  SceneViewport* scene_viewport = static_cast<SceneViewport*>(viewport);

  bool key_press_event_is_propagating = false;
  if (keyboard_event->repeat == false) {
    KeyPressEvent key_press_event(key);
    scene_viewport->scene()->ProcessEvent(&key_press_event);
    key_press_event_is_propagating = !key_press_event.is_propagating();
  }

  // Always prevent default action for the tab key
  if (key == Key::Tab()) {
    return true;
  }

  // Never prevent default action for CTRL+V or Insert key (Paste)
  if ((key == Key::V() && keyboard_event->ctrlKey) || key == Key::Insert()) {
    return false;
  }

  // Never prevent default action for CTRL+C (Copy)
  if (key == Key::C() && keyboard_event->ctrlKey) {
    return false;
  }

  return !key_press_event_is_propagating;
}

// TODO: the keypress event is actually deprecated and should be replaced by the keydown event. But there is no easy way
// to check whether
EM_BOOL HandleKeyPressEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* viewport) {
  SceneViewport* scene_viewport = static_cast<SceneViewport*>(viewport);

  // We do not want to send text input when a modifier key other than shift is pressed and
  // for some reason keyboard_event->key contains "Enter" when pressing the enter button
  if (keyboard_event->altKey == false && keyboard_event->ctrlKey == false && keyboard_event->metaKey == false &&
      std::strcmp("Enter", keyboard_event->key) != 0) {
    TextInputEvent text_input_event(keyboard_event->key);
    scene_viewport->scene()->ProcessEvent(&text_input_event);
    return !text_input_event.is_propagating();
  } else {
    return false;
  }
}

EM_BOOL HandleKeyUpEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* viewport) {
  const Key key = KEY_NAME_TO_KEY.at(keyboard_event->code);
  SetKeyState(key, false);

  SceneViewport* scene_viewport = static_cast<SceneViewport*>(viewport);
  KeyReleaseEvent key_release_event(key);
  scene_viewport->scene()->ProcessEvent(&key_release_event);
  return !key_release_event.is_propagating();
}

EM_BOOL HandleMouseMoveEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* viewport) {
  SceneViewport* scene_viewport = static_cast<SceneViewport*>(viewport);
  MouseMoveEvent mouse_move_event(
      scene_viewport, {static_cast<float>(mouse_event->targetX), static_cast<float>(mouse_event->targetY)},
      {static_cast<float>(mouse_event->movementX), static_cast<float>(mouse_event->movementY)});
  scene_viewport->scene()->ProcessEvent(&mouse_move_event);
  return !mouse_move_event.is_propagating();
}

EM_BOOL HandleMouseDownEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* viewport) {
  const MouseButton button = MouseButton{static_cast<uint8_t>(mouse_event->button)};
  SetMouseButtonState(button, true);

  SceneViewport* scene_viewport = static_cast<SceneViewport*>(viewport);
  MouseButtonPressEvent mouse_button_event(
      scene_viewport, {static_cast<float>(mouse_event->targetX), static_cast<float>(mouse_event->targetY)}, button);
  scene_viewport->scene()->ProcessEvent(&mouse_button_event);
  return false;
}

EM_BOOL HandleMouseUpEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* viewport) {
  const MouseButton button = MouseButton{static_cast<uint8_t>(mouse_event->button)};
  SetMouseButtonState(button, false);

  SceneViewport* scene_viewport = static_cast<SceneViewport*>(viewport);
  MouseButtonReleaseEvent mouse_button_event(
      scene_viewport, {static_cast<float>(mouse_event->targetX), static_cast<float>(mouse_event->targetY)}, button);
  scene_viewport->scene()->ProcessEvent(&mouse_button_event);
  return false;
}

#endif

}  // namespace ovis
