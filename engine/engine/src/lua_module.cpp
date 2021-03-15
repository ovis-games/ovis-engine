#include <sol/sol.hpp>

#include <ovis/engine/scene.hpp>
#include <ovis/engine/scene_object.hpp>

namespace ovis {

int LoadEngineModule(lua_State* l) {
  sol::state_view state(l);

  /// Engine module
  // @module ovis.engine
  // @usage local engine = require('ovis.engine')
  sol::table engine_module = state.create_table();

  Scene::RegisterType(&engine_module);
  SceneObject::RegisterType(&engine_module);

  return engine_module.push();
}

}  // namespace ovis