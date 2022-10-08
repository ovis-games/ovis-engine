#include "ovis/editor/editor_viewport.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>

#include "ovis/editor/json_helper.hpp"
#include "ovis/utils/json.hpp"
#include "ovis/rendering/clear_pass.hpp"
#include "ovis/rendering2d/renderer2d.hpp"
#include "ovis/editor/render_passes/transformation_tools_renderer.hpp"

namespace {
// emscripten::val document = emscripten::val::undefined();

// bool SetScene(const std::string& serialized_scene) {
//   using namespace ovis::editor;
//   if (EditorViewport::instance() == nullptr) {
//     return false;
//   }

//   document = emscripten::val::global("JSON").call<emscripten::val>("parse", serialized_scene);

//   ovis::SceneObject::ClearObjectTemplateChache();
//   return EditorViewport::instance()->scene()->Deserialize(ovis::json::parse(serialized_scene));
// }

// void Play() {
//   using namespace ovis::editor;
//   EditorViewport::instance()->scene()->Play();
// }

// void Stop() {
//   using namespace ovis::editor;
//   EditorViewport::instance()->scene()->Stop();
// }

}

namespace ovis {
namespace editor {

namespace {

std::uintptr_t CreateEditorViewport(const std::string& target) {
  return reinterpret_cast<uintptr_t>(new EditorViewport(target));
}

void DestroyEditorViewport(std::uintptr_t editor_viewport) {
  delete reinterpret_cast<EditorViewport*>(editor_viewport);
}

void SetViewportScene(std::uintptr_t editor_viewport, const emscripten::val& scene) {
  LogV("Update scene: {}", SerializeValue(scene));
  reinterpret_cast<EditorViewport*>(editor_viewport)->scene()->Deserialize(json::parse(SerializeValue(scene)));
}


// void SelectTransformType(int transform_type) {
//   EditorViewport::instance()->transformation_tools_controller()->SelectTransformationType(
//       static_cast<TransformationToolsController::TransformationType>(transform_type));
// }
// int GetTransformType() {
//   return static_cast<int>(EditorViewport::instance()->transformation_tools_controller()->transformation_type());
// }
// bool IsValidSceneObjectName(const std::string& name) {
//   return SceneObject::IsValidName(name);
// }
}

EMSCRIPTEN_BINDINGS(editor_viewport_module) {
  emscripten::function("createEditorViewport", &CreateEditorViewport);
  emscripten::function("destroyEditorViewport", &DestroyEditorViewport);
  emscripten::function("setViewportScene", &SetViewportScene);
  // emscripten::function("viewportPlay", &Play);
  // emscripten::function("viewportStop", &Stop);
  // emscripten::function("viewportSetScene", &SetScene);
  // emscripten::function("viewportSelectTransformType", &SelectTransformType);
  // emscripten::function("viewportGetTransformType", &GetTransformType);
  // emscripten::function("viewportIsValidSceneObjectName", &IsValidSceneObjectName);
}

EditorViewport::EditorViewport(std::string target)
    : CanvasViewport(std::move(target)),
      camera_controller_(this),
      transformation_tools_controller_(this),
      object_selection_controller_(this),
      event_callback_(emscripten::val::null()) {
  LogOnError(AddRenderPass<ClearPass>());
  LogOnError(AddRenderPass<Renderer2D>());
  LogOnError(AddRenderPass<SelectedObjectBoundingBox>());
  LogOnError(AddRenderPass<TransformationToolsRenderer>());

  SetCustomCameraMatrices(Matrix3x4::IdentityTransformation(),
                          Matrix4::FromOrthographicProjection(-10, 10, -10, 10, -10, 10));

  AddController(camera_controller());
  AddController(transformation_tools_controller());
  AddController(object_selection_controller());

  SetScene(&scene_);
  scene()->SetMainViewport(this);
}

void EditorViewport::SetEventCallback(emscripten::val event_callback) {
  LogD("Set event callback: {}", event_callback.typeOf().as<std::string>());
  if (event_callback.typeOf().as<std::string>() == "function") {
    event_callback_ = event_callback;
  }
}

void EditorViewport::SendEvent(emscripten::val event) {
  LogD("Send event: {}", SerializeValue(event));
  if (event_callback_.typeOf() == emscripten::val("function")) {
    event_callback_(event);
  }
}

void EditorViewport::Update(std::chrono::microseconds delta_time) {
  for (const auto& controller : controllers_) {
    controller->Update(delta_time);
  }
  if (scene()->is_playing()) {
    scene()->BeforeUpdate();
    scene()->Update(delta_time);
    scene()->AfterUpdate();
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
  // emscripten::val current = document;
  // size_t index;
  // while ((index = path.find('/')) != std::string_view::npos) {
  //   std::string_view section = path.substr(0, index);
  //   if (section.length() > 0) {
  //     current = current[std::string(section)];
  //   }
  //   path = path.substr(index + 1);
  // }
  // if (path.length() > 0) {
  //   current = current[std::string(path)];
  // }
  // return current;
  return emscripten::val::null();
}

}  // namespace editor
}  // namespace ovis
