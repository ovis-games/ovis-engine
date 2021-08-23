#include <ovis/editor_viewport/editor_viewport.hpp>
#include <ovis/editor_viewport/render_passes/transformation_tools_renderer.hpp>

#if OVIS_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>
#endif


namespace {
emscripten::val document = emscripten::val::undefined();

bool SetScene(const std::string& serialized_scene) {
  using namespace ovis::editor;
  if (EditorViewport::instance() == nullptr) {
    return false;
  }

  document = emscripten::val::global("JSON").call<emscripten::val>("parse", serialized_scene);

  return EditorViewport::instance()->scene()->Deserialize(ovis::json::parse(serialized_scene));
}
}

namespace ovis {
namespace editor {

namespace {
void SelectTransformType(int transform_type) {
  EditorViewport::instance()->transformation_tools_controller()->SelectTransformationType(
      static_cast<TransformationToolsController::TransformationType>(transform_type));
}
int GetTransformType() {
  return static_cast<int>(EditorViewport::instance()->transformation_tools_controller()->transformation_type());
}
}

EMSCRIPTEN_BINDINGS(editor_viewport_module) {
  emscripten::function("viewportSetScene", &SetScene);
  emscripten::function("viewportSelectTransformType", &SelectTransformType);
  emscripten::function("viewportGetTransformType", &GetTransformType);
}

EditorViewport* EditorViewport::instance_ = nullptr;

EditorViewport::EditorViewport()
    : Window(WindowDescription{}), camera_controller_(this), event_callback_(emscripten::val::null()) {
  SDL_assert(instance_ == nullptr);
  instance_ = this;

  AddRenderPass("ClearPass");
  AddRenderPass("SpriteRenderer");

  AddRenderPass(std::make_unique<SelectedObjectBoundingBox>());
  AddRenderPass(std::make_unique<TransformationToolsRenderer>());

  SetCustomCameraMatrices(Matrix3x4::IdentityTransformation(),
                          Matrix4::FromOrthographicProjection(-10, 10, -10, 10, -10, 10));

  AddController(camera_controller());
  AddController(transformation_tools_controller());
  AddController(object_selection_controller());
}

EditorViewport::~EditorViewport() {
  SDL_assert(instance_ == this);
  instance_ = nullptr;
}

void EditorViewport::SetEventCallback(emscripten::val event_callback) {
  LogD("Set event callback: {}", event_callback.typeOf().as<std::string>());
  if (event_callback.typeOf().as<std::string>() == "function") {
    event_callback_ = event_callback;
  }
}

void EditorViewport::SendEvent(emscripten::val event) {
  LogD("Send event: {}", event_callback_.typeOf().as<std::string>());
  if (event_callback_.typeOf() == emscripten::val("function")) {
    event_callback_(event);
  }
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
}

emscripten::val GetDocumentValueAtPath(std::string_view path) {
  emscripten::val current = document;
  size_t index;
  while ((index = path.find('/')) != std::string_view::npos) {
    std::string_view section = path.substr(0, index);
    if (section.length() > 0) {
      current = current[std::string(section)];
    }
    path = path.substr(index + 1);
  }
  if (path.length() > 0) {
    current = current[std::string(path)];
  }
  return current;
}

}  // namespace editor
}  // namespace ovis
