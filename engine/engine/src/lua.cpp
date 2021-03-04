#include <middleclass.hpp>

#include <ovis/engine/input.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/module.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/scene_controller.hpp>
#include <ovis/engine/script_scene_controller.hpp>

namespace ovis {

sol::state Lua::state;
EventHandler<void(const std::string&)> Lua::on_error;

void Lua::SetupEnvironment() {
  state.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::math, sol::lib::table);
  state.require_script("class", middleclass::SOURCE);

  state["log_error"] = [](const std::string& message) { LogE("{}", message); };
  state["log_warning"] = [](const std::string& message) { LogW("{}", message); };
  state["log_info"] = [](const std::string& message) { LogI("{}", message); };
  state["log_debug"] = [](const std::string& message) { LogD("{}", message); };
  state["log_verbose"] = [](const std::string& message) { LogV("{}", message); };

  state["OvisErrorHandler"] = [](const std::string& message) { on_error.Invoke(message); };
  sol::protected_function::set_default_handler(state["OvisErrorHandler"]);

  sol::usertype<vector2> vector2_type =
      state.new_usertype<vector2>("Vector2", sol::constructors<vector2(), vector2(float, float)>());
  vector2_type["x"] = &vector2::x;
  vector2_type["y"] = &vector2::y;
  vector2_type["__tostring"] = [](const vector3& vector) { return fmt::format("{}", vector); };

  auto vector3_factories = sol::factories([](sol::table table) { return vector3(table[1], table[2], table[3]); });
  sol::usertype<vector3> vector3_type = state.new_usertype<vector3>(
      "Vector3", sol::constructors<vector3(), vector3(float, float, float)>(), sol::call_constructor,
      sol::constructors<vector3(), vector3(float, float, float)>(), sol::meta_function::construct, vector3_factories);
  vector3_type["x"] = &vector3::x;
  vector3_type["y"] = &vector3::y;
  vector3_type["z"] = &vector3::z;
  vector3_type["__tostring"] = [](const vector3& vector) { return fmt::format("{}", vector); };

  vector3_type[sol::meta_function::multiplication] = [](const vector3& vector, float scalar) {
    return vector * scalar;
  };
  vector3_type[sol::meta_function::unary_minus] = [](const vector3& vector) { return -vector; };

  sol::usertype<vector4> vector4_type =
      state.new_usertype<vector4>("Vector4", sol::constructors<vector4(), vector4(float, float, float, float)>());
  vector4_type["x"] = &vector4::x;
  vector4_type["y"] = &vector4::y;
  vector4_type["z"] = &vector4::z;
  vector4_type["w"] = &vector4::w;
  vector4_type["__tostring"] = [](const vector4& vector) { return fmt::format("{}", vector); };

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

}  // namespace ovis
