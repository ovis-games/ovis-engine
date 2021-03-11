#include <ovis/core/lua_modules/register_modules.hpp>
#include <ovis/math/color.hpp>
#include <ovis/math/lua_modules/register_modules.hpp>
#include <ovis/math/vector.hpp>
#include <ovis/engine/input.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/module.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/scene_controller.hpp>
#include <ovis/engine/script_scene_controller.hpp>

namespace ovis {

sol::state Lua::state;
EventHandler<void(const std::string&)> Lua::on_error;

int LoadEngineModule(lua_State* l) {
  sol::state_view lua(l);

  sol::table engine_module = lua.create_table();
  engine_module["Scene"] = lua["require"]("ovis.engine.Scene");
  engine_module["SceneObject"] = lua["require"]("ovis.engine.SceneObject");
  engine_module["ScriptSceneController"] = lua["require"]("ovis.engine.ScriptSceneController");

  return engine_module.push();
}

void Lua::SetupEnvironment() {
  state.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::math, sol::lib::table,
                       sol::lib::package);
  state["OvisErrorHandler"] = [](const std::string& message) { on_error.Invoke(message); };
  sol::protected_function::set_default_handler(state["OvisErrorHandler"]);

  RegisterCoreLuaModules(state.lua_state());
  RegisterMathLuaModules(state.lua_state());
  
  state.require("ovis.engine.Scene", &Scene::LoadLuaModule);
  state.require("ovis.engine.SceneObject", &SceneObject::LoadLuaModule);
  state.require("ovis.engine.ScriptSceneController", &ScriptSceneController::LoadLuaModule);

  Module::RegisterToLua();
  SceneController::RegisterToLua();
}

sol::protected_function_result Lua::Execute(const std::string& code, const std::string& chunk_name) {
  return state.do_string(code, "=" + chunk_name);
}

sol::protected_function_result Lua::Execute(const std::string& code, const std::string& chunk_name,
                                            std::function<void(const std::string&)> error_handler) {
  auto subscription = on_error.Subscribe(error_handler);

  auto result = Execute(code, chunk_name);
  if (!result.valid()) {
    error_handler(result);
  }

  subscription.Unsubscribe();

  return result;
}

}  // namespace ovis
