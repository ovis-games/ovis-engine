#include "module_loader.hpp"

#include <sol/sol.hpp>

#include <ovis/core/log.hpp>
#include <ovis/math/lua_modules/register_modules.hpp>

namespace ovis {

void RegisterMathLuaModules(lua_State* l) {
  sol::state_view state(l);
  state.require("ovis.math.Vector3", LoadVector3Module);
  state.require("ovis.math", LoadMathModule);
}

int LoadMathModule(lua_State* l) {
  sol::state_view state(l);

  sol::table math_module = state.create_table();
  math_module["Vector3"] = state["require"]("ovis.math.Vector3");

  return math_module.push();
}

}  // namespace ovis