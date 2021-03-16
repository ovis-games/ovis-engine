#include <ovis/input/input.hpp>
#include <ovis/input/keyboard.hpp>
#include <sol/sol.hpp>

namespace ovis {

int LoadInputModule(lua_State* l) {
  sol::state_view state(l);

  /// Engine module
  // @module ovis.engine
  // @usage local engine = require('ovis.engine')
  sol::table input_module = state.create_table();

  Input::RegisterType(&input_module);
  Key::RegisterType(&input_module);

  return input_module.push();
}

}  // namespace ovis