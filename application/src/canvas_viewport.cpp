#include "ovis/application/canvas_viewport.hpp"

#if OVIS_EMSCRIPTEN

#include <emscripten/html5.h>

#include "ovis/input/emscripten_callbacks.hpp"
#include "ovis/application/canvas_viewport.hpp"

namespace ovis {

CanvasViewport::CanvasViewport(const char* target) : target_(target) {
  emscripten_set_keydown_callback(target_.c_str(), static_cast<SceneViewport*>(this), 0, &HandleKeyDownEvent);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, static_cast<SceneViewport*>(this), 0,
                                &HandleKeyUpEvent);
  emscripten_set_keypress_callback(target_.c_str(), static_cast<SceneViewport*>(this), 0, &HandleKeyPressEvent);

  emscripten_set_mousemove_callback(target_.c_str(), static_cast<SceneViewport*>(this), 0, &HandleMouseMoveEvent);
  emscripten_set_mousedown_callback(target_.c_str(), static_cast<SceneViewport*>(this), 0, &HandleMouseDownEvent);
  emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, static_cast<SceneViewport*>(this), 0,
                                  &HandleMouseUpEvent);
  // emscripten_set_mouseenter_callback("canvas", nullptr, 0, HandleMouseEvent);
  // emscripten_set_mouseleave_callback("canvas", nullptr, 0, HandleMouseEvent);
  emscripten_set_wheel_callback(target_.c_str(), static_cast<SceneViewport*>(this), 0, &HandleWheelEvent);
  emscripten_set_blur_callback(target_.c_str(), static_cast<SceneViewport*>(this), 0, &HandleBlurEvent);
}

CanvasViewport::~CanvasViewport() {
  emscripten_set_keydown_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_keyup_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_keypress_callback(target_.c_str(), nullptr, 0, nullptr);

  emscripten_set_mousemove_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_mousedown_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, 0, nullptr);
  emscripten_set_mouseenter_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_mouseleave_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_wheel_callback(target_.c_str(), nullptr, 0, nullptr);
  emscripten_set_blur_callback(target_.c_str(), nullptr, 0, nullptr);
}

}  // namespace ovis

#endif
