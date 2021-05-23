#include <cstring>

#if OVIS_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/val.h>
#include <ovis/player/loading_controller.hpp>
#endif

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/rendering2d/rendering2d_module.hpp>
#include <ovis/application/application.hpp>
#include <ovis/application/window.hpp>

#if OVIS_EMSCRIPTEN
extern "C" {

void EMSCRIPTEN_KEEPALIVE QuitGame() {
  SDL_Event sdl_event;
  sdl_event.type = SDL_QUIT;
  SDL_PushEvent(&sdl_event);
}
}
#endif

// Usage: ovis-player backend_url project_id
int main(int argc, char* argv[]) {
  using namespace ovis;

  Log::AddListener(ConsoleLogger);

#if OVIS_EMSCRIPTEN
  if (argc != 4) {
    LogE("Invalid number of arguments to player");
    return -1;
  }

  std::string_view backend_prefix = argv[1];
  std::string_view project_id = argv[2];
  std::string_view package_type = argv[3];

#endif

  Init();

#if OVIS_EMSCRIPTEN
  SetEngineAssetsDirectory("/ovis_assets");
#endif

  Window window(WindowDescription{});
  window.AddRenderPass("ClearPass");
  window.AddRenderPass("SpriteRenderer");

#if OVIS_EMSCRIPTEN
  window.scene()->AddController<player::LoadingController>(backend_prefix, project_id, package_type);
#endif

  Run();

  Quit();
  return 0;
}

