#include <ovis/editor_viewport/editor_viewport.hpp>

#if OVIS_EMSCRIPTEN
#include <emscripten/html5.h>
#endif

namespace ovis {
namespace editor {

EditorViewport::EditorViewport() : Window(WindowDescription{}), camera_controller_(this) {
  AddRenderPass("ClearPass");
  AddRenderPass("SpriteRenderer");

  SetCustomCameraMatrices(Matrix3x4::IdentityTransformation(),
                          Matrix4::FromOrthographicProjection(-10, 10, -10, 10, -10, 10));

  AddController(&camera_controller_);
}

void EditorViewport::Update(std::chrono::microseconds delta_time) {
#if OVIS_EMSCRIPTEN
  double canvas_css_width;
  double canvas_css_height;
  int canvas_width;
  int canvas_height;
  if (emscripten_get_element_css_size("canvas", &canvas_css_width, &canvas_css_height) == EMSCRIPTEN_RESULT_SUCCESS &&
      emscripten_get_canvas_element_size("canvas", &canvas_width, &canvas_height) == EMSCRIPTEN_RESULT_SUCCESS &&
      (canvas_css_width > 0 && canvas_css_height > 0) &&
      (canvas_width != static_cast<int>(canvas_css_width) || canvas_height != static_cast<int>(canvas_css_height))) {
    LogV("Resize editor viewport from {}x{} to {}x{}", canvas_width, canvas_height, canvas_css_width, canvas_css_height);
    Resize(static_cast<int>(canvas_css_width), static_cast<int>(canvas_css_height));
  }
#endif
  Window::Update(delta_time);
  for (const auto& controller : controllers_) {
    controller->Update(delta_time);
  }
}

void EditorViewport::ProcessEvent(Event* event) {
  for (const auto& controller : controllers_) {
    controller->ProcessEvent(event);
    if (!event->is_propagating()) {
      return;
    }
  }

  scene()->ProcessEvent(event);
}
  
void EditorViewport::AddController(ViewportController* controller) {
  controllers_.push_back(controller);
  controller->viewport_ = this;
}

}  // namespace editor
}  // namespace ovis
