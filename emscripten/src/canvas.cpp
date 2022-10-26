#include "ovis/application/canvas_viewport.hpp"

#if OVIS_EMSCRIPTEN

#include "emscripten/html5.h"
#include "emscripten/html5_webgl.h"

#include "ovis/input/emscripten_callbacks.hpp"
#include "ovis/application/canvas_viewport.hpp"

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

CanvasViewport::CanvasViewport(std::string target)
    : target_(std::move(target)), webgl_context_(CreateWebGLContext(target_.c_str())), graphics_context_(GetDimensions()) {

  // Explicitely pass a SceneViewport pointer as user data as the callbacks expect a SceneViewport not a CanvasViewport.
  // Generally the actual pointer should be the same but better be safe than sorry.
  SceneViewport* scene_viewport = this;
  emscripten_set_keydown_callback(target_.c_str(), scene_viewport, 0, &HandleKeyDownEvent);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, scene_viewport, 0, &HandleKeyUpEvent);
  emscripten_set_keypress_callback(target_.c_str(), scene_viewport, 0, &HandleKeyPressEvent);

  emscripten_set_mousemove_callback(target_.c_str(), scene_viewport, 0, &HandleMouseMoveEvent);
  emscripten_set_mousedown_callback(target_.c_str(), scene_viewport, 0, &HandleMouseDownEvent);
  emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, scene_viewport, 0, &HandleMouseUpEvent);
  // emscripten_set_mouseenter_callback("canvas", nullptr, 0, HandleMouseEvent);
  // emscripten_set_mouseleave_callback("canvas", nullptr, 0, HandleMouseEvent);
  emscripten_set_wheel_callback(target_.c_str(), scene_viewport, 0, &HandleWheelEvent);
  emscripten_set_blur_callback(target_.c_str(), scene_viewport, 0, &HandleBlurEvent);

  CheckForDimensionChanges();
  SetGraphicsContext(&graphics_context_);
}

CanvasViewport::~CanvasViewport() {
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

  ClearResources();
  emscripten_webgl_destroy_context(webgl_context_);
}

Vector2 CanvasViewport::GetDimensions() const {
  int canvas_width = 0;
  int canvas_height = 0;
  const auto result = emscripten_get_canvas_element_size(target_.c_str(), &canvas_width, &canvas_height);
  assert(result == EMSCRIPTEN_RESULT_SUCCESS);
  return { static_cast<float>(canvas_width), static_cast<float>(canvas_height) };
}

RenderTargetConfiguration* CanvasViewport::GetDefaultRenderTargetConfiguration() {
  return graphics_context_.default_render_target_configuration();
}

void CanvasViewport::Render() {
  CheckForDimensionChanges();
  emscripten_webgl_make_context_current(webgl_context_);
  RenderingViewport::Render();
  // TODO: should we flush here?
}

void CanvasViewport::CheckForDimensionChanges() {
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

}  // namespace ovis

#endif
