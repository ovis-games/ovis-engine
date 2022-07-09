#include "ovis/editor/scripting.hpp"

#include <emscripten/bind.h>

#include "ovis/vm/script_parser.hpp"

namespace ovis {

namespace {

bool parseGameModuleScripts() {
  return false;
}

EMSCRIPTEN_BINDINGS(editor_module) {
  emscripten::function("parseGameModuleScripts", parseGameModuleScripts);
}

}

}
