#include <sol/sol.hpp>

#include <ovis/core/core_module.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/math/math_module.hpp>
#include <ovis/scene/scene.hpp>
#include <ovis/scene/scene_controller.hpp>
#include <ovis/scene/scene_object.hpp>
#include <ovis/scene/scene_object_component.hpp>
#include <ovis/scene/transform.hpp>

namespace ovis {

namespace {
int LoadSceneModule(lua_State* l) {
  sol::state_view state(l);

  /// Engine module
  // @module ovis.engine
  // @usage local engine = require('ovis.engine')
  sol::table scene_module = state.create_table();

  Scene::RegisterType(&scene_module);
  SceneObject::RegisterType(&scene_module);
  SceneObjectComponent::RegisterType(&scene_module);
  Transform::RegisterType(&scene_module);
  SceneController::RegisterType(&scene_module);

  return scene_module.push();
}
}  // namespace

bool LoadSceneModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    SceneObjectComponent::Register("Transform", []() { return std::make_unique<Transform>(); });

    LoadCoreModule();
    LoadMathModule();
    lua.require("ovis.scene", &LoadSceneModule);
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis