#include <cstring>

#include "emscripten.h"
#include "emscripten/bind.h"
#include "emscripten/val.h"

#include "ovis/utils/log.hpp"
#include "ovis/core/asset_library.hpp"
#include "ovis/core/application.hpp"
#include "ovis/editor/editor_viewport.hpp"

using namespace ovis;
using namespace ovis::editor;

emscripten::val log_callback = emscripten::val::undefined();
void SetLogCallback(emscripten::val callback) {
  log_callback = callback;
}

EMSCRIPTEN_BINDINGS(my_module) {
  function("setLogCallback", SetLogCallback);
}

// Usage: ovis-player backend_url project_id
int main(int argc, char* argv[]) {
  using namespace ovis;
  using namespace ovis::editor;

  Log::AddListener(ConsoleLogger);
  Log::AddListener([](LogLevel level, const std::string& text) {
    if (log_callback.typeOf() == emscripten::val("function")) {
      log_callback(emscripten::val(static_cast<int>(level)), emscripten::val(text));
    }
  });

  SetEngineAssetsDirectory("/ovis_assets");
  RunApplicationLoop();
}
