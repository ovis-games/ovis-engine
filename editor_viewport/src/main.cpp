#include <cstring>

#if OVIS_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/val.h>
#endif

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/rendering/clear_pass.hpp>
#include <ovis/application/application.hpp>
#include <ovis/application/window.hpp>

ovis::Scene* scene = nullptr;

#if OVIS_EMSCRIPTEN
extern "C" {

void EMSCRIPTEN_KEEPALIVE OvisEditorViewport_Quit() {
  scene = nullptr;
  SDL_Event sdl_event;
  sdl_event.type = SDL_QUIT;
  SDL_PushEvent(&sdl_event);
}

bool EMSCRIPTEN_KEEPALIVE OvisEditorViewport_SetScene(const char* serialized_scene) {
  if (scene == nullptr) {
    return false;
  }

  return scene->Deserialize(ovis::json::parse(serialized_scene));
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
#endif

// Usage: ovis-player backend_url project_id
int main(int argc, char* argv[]) {
  using namespace ovis;

  Log::AddListener(ConsoleLogger);
  Log::AddListener([](LogLevel level, const std::string& text) {
    emscripten::val::global("window").call<void>("ovis_log", emscripten::val(GetLogLevelString(level)),
                                                 emscripten::val(text.c_str()));
  });

  Init();

#if OVIS_EMSCRIPTEN
  SetEngineAssetsDirectory("/ovis_assets");
#endif

  Window window(WindowDescription{});
  window.AddRenderPass("ClearPass");
  window.AddRenderPass("SpriteRenderer");

  scene = window.scene();

  window.SetCustomCameraMatrices(
    Matrix3x4::IdentityTransformation(),
    Matrix4::FromOrthographicProjection(-10, 10, -10, 10, -10, 10)
  );

  Run();

  scene = nullptr;

  Quit();
}
