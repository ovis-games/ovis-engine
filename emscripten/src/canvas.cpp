#include "ovis/emscripten/canvas.hpp"

#include "emscripten/html5.h"
#include "emscripten/html5_webgl.h"

#include "ovis/emscripten/key_codes.hpp"

namespace ovis {

namespace {

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE CreateWebGLContext(const char* target) {
  EmscriptenWebGLContextAttributes context_attributes;
  emscripten_webgl_init_context_attributes(&context_attributes);
  auto context = emscripten_webgl_create_context(target, &context_attributes);
  if (context < 0) {
    LogE("Failed to create WebGL context: {}", context);
  } else {
    emscripten_webgl_make_context_current(context);
    LogV("Successfully to create WebGL context");
  }
  return context;
}

}

Canvas::Canvas(std::string target)
    : target_(std::move(target)), webgl_context_(CreateWebGLContext(target_.c_str())), graphics_context_(GetDimensions()) {

  emscripten_set_keydown_callback(target_.c_str(), this, 0, &HandleKeyDownEvent);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, 0, &HandleKeyUpEvent);

  emscripten_set_mousemove_callback(target_.c_str(), this, 0, &HandleMouseMoveEvent);
  emscripten_set_mousedown_callback(target_.c_str(), this, 0, &HandleMouseDownEvent);
  emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, 0, &HandleMouseUpEvent);
  // emscripten_set_mouseenter_callback("canvas", nullptr, 0, HandleMouseEvent);
  // emscripten_set_mouseleave_callback("canvas", nullptr, 0, HandleMouseEvent);
  emscripten_set_wheel_callback(target_.c_str(), this, 0, &HandleWheelEvent);
  emscripten_set_blur_callback(target_.c_str(), this, 0, &HandleBlurEvent);

  CheckForDimensionChanges();
}

Canvas::~Canvas() {
  emscripten_set_keydown_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, 0, nullptr);
  emscripten_set_keypress_callback(target_.c_str(), nullptr, 0, nullptr);

  emscripten_set_mousemove_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_mousedown_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, 0, nullptr);
  // emscripten_set_mouseenter_callback(target_.c_str(), nullptr, 0, nullptr);
  // emscripten_set_mouseleave_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_wheel_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_blur_callback(target_.c_str(), nullptr, 0, nullptr);

  emscripten_webgl_destroy_context(webgl_context_);
}

Vector2 Canvas::GetDimensions() const {
  int canvas_width = 0;
  int canvas_height = 0;
  const auto result = emscripten_get_canvas_element_size(target_.c_str(), &canvas_width, &canvas_height);
  assert(result == EMSCRIPTEN_RESULT_SUCCESS);
  return { static_cast<float>(canvas_width), static_cast<float>(canvas_height) };
}

RenderTargetConfiguration* Canvas::GetDefaultRenderTargetConfiguration() {
  return graphics_context_.default_render_target_configuration();
}

// void Canvas::Render() {
//   CheckForDimensionChanges();
//   emscripten_webgl_make_context_current(webgl_context_);
//   RenderingViewport::Render();
//   // TODO: should we flush here?
// }

void Canvas::CheckForDimensionChanges() {
  double canvas_css_width;
  double canvas_css_height;
  const Vector2 canvas_dimensions = GetDimensions();
  if (emscripten_get_element_css_size(target_.c_str(), &canvas_css_width, &canvas_css_height) == EMSCRIPTEN_RESULT_SUCCESS &&
      (canvas_css_width > 0 && canvas_css_height > 0) &&
      (canvas_dimensions.x != static_cast<int>(canvas_css_width) || canvas_dimensions.y != static_cast<int>(canvas_css_height))) {
    LogV("Resize editor viewport from {}x{} to {}x{}", canvas_dimensions.x,canvas_dimensions.y, canvas_css_width, canvas_css_height);
    emscripten_set_canvas_element_size(target_.c_str(), canvas_css_width, canvas_css_height);
    graphics_context_.SetFramebufferSize(canvas_css_width, canvas_css_height);
  }
}

EM_BOOL Canvas::HandleWheelEvent(int event_type, const EmscriptenWheelEvent* wheel_event, void* user_data) {
  auto* canvas = static_cast<Canvas*>(user_data);

  canvas->input_event_emitter_.PushEvent(
    MouseWheelEvent{
      .delta = {static_cast<float>(-wheel_event->deltaX), static_cast<float>(-wheel_event->deltaY)},
      .mode = static_cast<MouseWheelDeltaMode>(wheel_event->deltaMode),
    }
  );

  return false;
}

EM_BOOL Canvas::HandleKeyDownEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* user_data) {
  const Key key = EMSCRIPTEN_CODE_TO_KEY.at(keyboard_event->code);
  SetKeyState(key, true);

  auto canvas = static_cast<Canvas*>(user_data);

  canvas->input_event_emitter_.PushEvent(
    KeyPressEvent {
      .key = key
    }
  );

  return false;

  // bool key_press_event_is_propagating = false;
  // if (keyboard_event->repeat == false) {
  //   KeyPressEvent key_press_event(key);
  //   scene_canvas->ProcessEvent(&key_press_event);
  //   key_press_event_is_propagating = key_press_event.is_propagating();
  // }

  // // Always prevent default action for the tab key
  // if (key == Key::Tab()) {
  //   return true;
  // }

  // const bool default_modifier_pressed =
  //     GetPlatform() == Platform::MACOS ? keyboard_event->metaKey : keyboard_event->ctrlKey;

  // // Never prevent default action for CTRL+V or Insert key (Paste)
  // if ((key == Key::V() && default_modifier_pressed) || key == Key::Insert()) {
  //   return false;
  // }

  // // Never prevent default action for CTRL+C (Copy) and CTRL+X (Cut)
  // if ((key == Key::C() || key == Key::X()) && default_modifier_pressed) {
  //   return false;
  // }

  // return !key_press_event_is_propagating;
}

// EM_BOOL Canvas::HandleKeyPressEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* user_data) {
//   auto canvas = static_cast<Canvas*>(user_data);

//   // We do not want to send text input when a modifier key other than shift is pressed and
//   // for some reason keyboard_event->key contains "Enter" when pressing the enter button
//   if (keyboard_event->altKey == false && keyboard_event->ctrlKey == false && keyboard_event->metaKey == false &&
//       std::strcmp("Enter", keyboard_event->key) != 0) {
//     TextInputEvent text_input_event(keyboard_event->key);
//     scene_canvas->ProcessEvent(&text_input_event);
//     return !text_input_event.is_propagating();
//   } else {
//     return false;
//   }
// }

EM_BOOL Canvas::HandleKeyUpEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* user_data) {
  const Key key = EMSCRIPTEN_CODE_TO_KEY.at(keyboard_event->code);
  SetKeyState(key, false);

  auto canvas = static_cast<Canvas*>(user_data);
  canvas->input_event_emitter_.PushEvent(
    KeyReleaseEvent {
      .key = key
    }
  );
  return false;
}

EM_BOOL Canvas::HandleMouseMoveEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* user_data) {
  auto canvas = static_cast<Canvas*>(user_data);
  const float x = mouse_event->targetX;
  const float y = mouse_event->targetY;
  const float dx = mouse_event->movementX;
  const float dy = mouse_event->movementY;
  canvas->input_event_emitter_.PushEvent(
    MouseMoveEvent {
      .screen_space_position = {x, y},
      .relative_screen_space_position = {dx, dy},
    }
  );
  return true;
}

EM_BOOL Canvas::HandleMouseDownEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* user_data) {
  const float x = mouse_event->targetX;
  const float y = mouse_event->targetY;
  const MouseButton button = MouseButton{static_cast<uint8_t>(mouse_event->button)};
  SetMouseButtonState(button, true);

  auto canvas = static_cast<Canvas*>(user_data);
  canvas->input_event_emitter_.PushEvent(
    MouseButtonPressEvent {
      .screen_space_position = {x, y},
      .button = button,
    }
  );
  return false;
}

EM_BOOL Canvas::HandleMouseUpEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* user_data) {
  const float x = mouse_event->targetX;
  const float y = mouse_event->targetY;
  const MouseButton button = MouseButton{static_cast<uint8_t>(mouse_event->button)};
  SetMouseButtonState(button, false);

  auto canvas = static_cast<Canvas*>(user_data);
  canvas->input_event_emitter_.PushEvent(
    MouseButtonReleaseEvent {
      .screen_space_position = {x, y},
      .button = button,
    }
  );
  return false;
}

EM_BOOL Canvas::HandleBlurEvent(int event_type, const EmscriptenFocusEvent* focus_event, void* user_data) {
  auto canvas = static_cast<Canvas*>(user_data);
  for (Key key : Keys()) {
    if (IsKeyPressed(key)) {
      SetKeyState(key, false);
      canvas->input_event_emitter_.PushEvent(
        KeyReleaseEvent {
          .key = key
        }
      );
    }
  }
  return false;
}

}  // namespace ovis
