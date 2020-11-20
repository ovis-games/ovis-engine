#include <emscripten.h>

#include "editor_module.hpp"
#include "editor_window.hpp"
#include "global.hpp"
#include <ovis/rendering2d/rendering2d_module.hpp>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/log.hpp>

#include <ovis/base/base_module.hpp>
#include <ovis/engine/engine.hpp>

// Usage: ovis-editor backend_url project_id authentication_token
int main(int argc, char* argv[]) {
  ovis::Log::AddListener(ovis::ConsoleLogger);

  if (argc != 4) {
    ovis::LogE("Invalid number of arguments to editor");
    return -1;
  }

  ove::backend_url = argv[1];
  ove::project_id = argv[2];
  ove::authentication_token = argv[3];

  ovis::Init();
  ovis::SetEngineAssetsDirectory("/ovis_assets");
  ovis::LoadModule<ovis::BaseModule>();
  ovis::LoadModule<ovis::Rendering2DModule>();
  ovis::LoadModule<ove::EditorModule>();

  ove::EditorWindow editor_window;

  ovis::Run();

  ovis::Quit();
  return 0;
}