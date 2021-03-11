#include <middleclass.hpp>

#include <ovis/math/vector.hpp>
#include <ovis/math/color.hpp>
#include <ovis/math/lua_modules/register_modules.hpp>
#include <ovis/engine/input.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/module.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/scene_controller.hpp>
#include <ovis/engine/script_scene_controller.hpp>

namespace ovis {

sol::state Lua::state;
EventHandler<void(const std::string&)> Lua::on_error;

void RegisterVector2(sol::state& state);
// void RegisterVector3(sol::state& state);
void RegisterColor(sol::state& state);

int foo(lua_State* l) {
  sol::state_view state(l);
  sol::table t = state.create_table();

  t["foo"] = 1;

  return t.push();
}

void Lua::SetupEnvironment() {
  state.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::package);
  state.require_script("class", middleclass::SOURCE);

  state["log_error"] = [](const std::string& message) { LogE("{}", message); };
  state["log_warning"] = [](const std::string& message) { LogW("{}", message); };
  state["log_info"] = [](const std::string& message) { LogI("{}", message); };
  state["log_debug"] = [](const std::string& message) { LogD("{}", message); };
  state["log_verbose"] = [](const std::string& message) { LogV("{}", message); };

  state["OvisErrorHandler"] = [](const std::string& message) { on_error.Invoke(message); };
  sol::protected_function::set_default_handler(state["OvisErrorHandler"]);

  RegisterMathLuaModules(state.lua_state());

  // RegisterVector2(state);
  // RegisterVector3(state);
  // RegisterColor(state);

  // auto Vector3_factories = sol::factories([](sol::table table) { return Vector3(table[1], table[2], table[3]); });
  // sol::usertype<Vector3> Vector3_type = state.new_usertype<Vector3>(
  //     "Vector3", sol::constructors<Vector3(), Vector3(float, float, float)>(), sol::call_constructor,
  //     sol::constructors<Vector3(), Vector3(float, float, float)>(), sol::meta_function::construct,
  //     Vector3_factories);
  // Vector3_type["x"] = &Vector3::x;
  // Vector3_type["y"] = &Vector3::y;
  // Vector3_type["z"] = &Vector3::z;
  // Vector3_type["__tostring"] = [](const Vector3& vector) { return fmt::format("{}", vector); };

  // Vector3_type[sol::meta_function::multiplication] = [](const Vector3& vector, float scalar) {
  //   return vector * scalar;
  // };
  // Vector3_type[sol::meta_function::unary_minus] = [](const Vector3& vector) { return -vector; };

  // sol::usertype<vector4> vector4_type =
  //     state.new_usertype<vector4>("Vector4", sol::constructors<vector4(), vector4(float, float, float, float)>());
  // vector4_type["x"] = &vector4::x;
  // vector4_type["y"] = &vector4::y;
  // vector4_type["z"] = &vector4::z;
  // vector4_type["w"] = &vector4::w;
  // vector4_type["__tostring"] = [](const vector4& vector) { return fmt::format("{}", vector); };

  // sol::usertype<Scene> scene_type = state.new_usertype<Scene>("Scene");
  // scene_type["CreateObject"] = static_cast<SceneObject* (Scene::*)(const std::string&)>(&Scene::CreateObject);
  // scene_type["DeleteObject"] = &Scene::DeleteObject;
  // scene_type["GetObject"] = &Scene::GetObject;

  Module::RegisterToLua();
  SceneObject::RegisterToLua();
  SceneController::RegisterToLua();
  ScriptSceneController::RegisterToLua();
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

// clang-format off




}  // namespace ovis
