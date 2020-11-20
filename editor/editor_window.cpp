#include "editor_window.hpp"

#include <emscripten/html5.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "loading_controller.hpp"

#include <ovis/core/log.hpp>

#include <ovis/engine/scene.hpp>
#include <ovis/engine/window.hpp>

namespace ove {

namespace {

ovis::WindowDescription CreateWindowDescription() {
  ovis::WindowDescription window_description;

  window_description.title = "Ovis Editor";
  window_description.resource_search_paths = {"/resources/", "/assets/"};
  window_description.scene_controllers = {"ImGui", "LoadingController"};
  window_description.render_passes = {"ImGui"};

  double canvas_css_width;
  double canvas_css_height;
  if (emscripten_get_element_css_size("canvas", &canvas_css_width, &canvas_css_height) == EMSCRIPTEN_RESULT_SUCCESS) {
    window_description.width = static_cast<int>(canvas_css_width);
    window_description.height = static_cast<int>(canvas_css_height);
  }

  return window_description;
}

}  // namespace

EditorWindow* EditorWindow::instance_ = nullptr;

EditorWindow::EditorWindow() : ovis::Window(CreateWindowDescription()) {
  SDL_assert(instance_ == nullptr);
  instance_ = this;
}

void EditorWindow::Update(std::chrono::microseconds delta_time) {
  double canvas_css_width;
  double canvas_css_height;
  int canvas_width;
  int canvas_height;
  if (emscripten_get_element_css_size("canvas", &canvas_css_width, &canvas_css_height) == EMSCRIPTEN_RESULT_SUCCESS &&
      emscripten_get_canvas_element_size("canvas", &canvas_width, &canvas_height) == EMSCRIPTEN_RESULT_SUCCESS &&
      (canvas_width != static_cast<int>(canvas_css_width) || canvas_height != static_cast<int>(canvas_css_height))) {
    ovis::LogV("css: {}x{} vs real: {}x{}", canvas_css_width, canvas_css_height, canvas_width, canvas_height);
    // emscripten_set_canvas_element_size("canvas",
    // static_cast<int>(canvas_css_width),
    // static_cast<int>(canvas_css_height));
    Resize(static_cast<int>(canvas_css_width), static_cast<int>(canvas_css_height));
  }

  Window::Update(delta_time);

  auto loading_controller = scene()->GetController<LoadingController>("LoadingController");
  if (loading_controller != nullptr && loading_controller->is_finished()) {
    scene()->RemoveController("LoadingController");
    scene()->AddController("EditorWindowController");
  }
}

}  // namespace ove