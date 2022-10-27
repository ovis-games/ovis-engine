#pragma once

#include "emscripten/html5_webgl.h"

#include "ovis/utils/all.hpp"
#include "ovis/core/scene.hpp"
#include "ovis/graphics/graphics_context.hpp"
#include "ovis/input/input_event_emitter.hpp"

namespace ovis {

class Canvas {
 public:
  Canvas(std::string target);
  ~Canvas();

  std::string_view target() const { return target_; }

  Vector2 GetDimensions() const;

  RenderTargetConfiguration* GetDefaultRenderTargetConfiguration();

 private:
  // Checks if the css dimensions have changed and resizes the framebuffer accordingly.
  void CheckForDimensionChanges();

  std::string target_;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context_;
  GraphicsContext graphics_context_;

  InputEventEmitter input_event_emitter_;

  static EM_BOOL HandleWheelEvent(int event_type, const EmscriptenWheelEvent* wheel_event, void* canvas);
  static EM_BOOL HandleKeyDownEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* canvas);
  static EM_BOOL HandleKeyUpEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* canvas);
  static EM_BOOL HandleMouseMoveEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* canvas);
  static EM_BOOL HandleMouseDownEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* canvas);
  static EM_BOOL HandleMouseUpEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* canvas);
  static EM_BOOL HandleBlurEvent(int event_type, const EmscriptenFocusEvent* focus_event, void* canvas);
};

}  // namespace ovis
