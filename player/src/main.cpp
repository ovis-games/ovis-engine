#include <cstring>

#include <emscripten.h>
#include <emscripten/val.h>
#include <ovis/base/base_module.hpp>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/log.hpp>
#include <ovis/engine/engine.hpp>
#include <ovis/engine/window.hpp>
#include <ovis/rendering2d/rendering2d_module.hpp>
#include <ovis/player/loading_controller.hpp>

extern "C" {

void EMSCRIPTEN_KEEPALIVE QuitGame() {
  SDL_Event sdl_event;
  sdl_event.type = SDL_QUIT;
  SDL_PushEvent(&sdl_event);
}

}

// Usage: ovis-player backend_url project_id
int main(int argc, char* argv[]) {
  using namespace ovis;

  Log::AddListener(ConsoleLogger);

  if (argc != 3) {
    LogE("Invalid number of arguments to player");
    return -1;
  }

  // backend_url = argv[1];
  // project_id = argv[2];

  Init();
  SetEngineAssetsDirectory("/ovis_assets");
  LoadModule<BaseModule>();
  LoadModule<Rendering2DModule>();

  Window window(WindowDescription{});

  window.scene()->AddController<player::LoadingController>(true);

  Run();

  Quit();
  return 0;
}