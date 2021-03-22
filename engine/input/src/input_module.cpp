#include <sol/sol.hpp>

#include <ovis/core/core_module.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/input/input_module.hpp>
#include <ovis/input/key.hpp>

namespace ovis {

int LoadInputModule(lua_State* l) {
  sol::state_view state(l);

  /// This module provides core components of the engine.
  // @module ovis.input
  // @usage local input = require('ovis.input')
  sol::table input_module = state.create_table();

  Key::RegisterType(&input_module);

  return input_module.push();
}

bool LoadInputModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    LoadCoreModule();
    lua.require("ovis.input", &LoadInputModule);
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis