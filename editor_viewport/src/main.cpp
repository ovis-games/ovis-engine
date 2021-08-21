#include <cstring>

#if OVIS_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#endif

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/rendering/clear_pass.hpp>
#include <ovis/application/application.hpp>
#include <ovis/application/window.hpp>

#include <ovis/editor_viewport/editor_viewport.hpp>

#if OVIS_EMSCRIPTEN
extern "C" {
using namespace ovis;
using namespace ovis::editor;

void EMSCRIPTEN_KEEPALIVE OvisEditorViewport_Quit() {
  SDL_Event sdl_event;
  sdl_event.type = SDL_QUIT;
  SDL_PushEvent(&sdl_event);
}

bool EMSCRIPTEN_KEEPALIVE OvisEditorViewport_SetScene(const char* serialized_scene) {
  if (EditorViewport::instance() == nullptr) {
    return false;
  }

  return EditorViewport::instance()->scene()->Deserialize(ovis::json::parse(serialized_scene));
}

int EMSCRIPTEN_KEEPALIVE OvisEditorViewport_GetRegisteredSceneObjectComponentCount() {
  return ovis::SceneObjectComponent::GetRegisteredFactoriesCount();
}

const char* EMSCRIPTEN_KEEPALIVE OvisEditorViewport_GetRegisteredSceneObjectComponentId(int index) {
  if (index >= OvisEditorViewport_GetRegisteredSceneObjectComponentCount()) {
    return nullptr;
  } else {
    return std::next(ovis::SceneObjectComponent::registered_ids().begin(), index)->c_str();
  }
}

}

void SetEventCallback(emscripten::val callback) {
  EditorViewport::instance()->SetEventCallback(callback);
}

EMSCRIPTEN_BINDINGS(editor_viewport_module) {
  function("viewportSetEventCallback", &SetEventCallback);
}

#endif

// Usage: ovis-player backend_url project_id
int main(int argc, char* argv[]) {
  using namespace ovis;
  using namespace ovis::editor;

  Log::AddListener(ConsoleLogger);
  Log::AddListener([](LogLevel level, const std::string& text) {
    emscripten::val::global("window").call<void>("ovis_log", emscripten::val(GetLogLevelString(level)),
                                                 emscripten::val(text.c_str()));
  });

  Init();

#if OVIS_EMSCRIPTEN
  SetEngineAssetsDirectory("/ovis_assets");
#endif

  EditorViewport viewport;

  Run();

  Quit();
}
