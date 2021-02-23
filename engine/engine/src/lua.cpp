#include <middleclass.hpp>

#include <ovis/engine/lua.hpp>
#include <ovis/engine/module.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/scene_controller.hpp>
#include <ovis/engine/script_scene_controller.hpp>

namespace ovis {

sol::state Lua::state;
Event<void(const std::string&)> Lua::on_error;

void Lua::SetupEnvironment() {
  state.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::math);
  state.require_script("class", middleclass::SOURCE);

  state["LogE"] = [](const std::string& message) { LogE("{}", message); };
  state["LogW"] = [](const std::string& message) { LogW("{}", message); };
  state["LogI"] = [](const std::string& message) { LogI("{}", message); };
  state["LogD"] = [](const std::string& message) { LogD("{}", message); };
  state["LogV"] = [](const std::string& message) { LogV("{}", message); };

  state["OvisErrorHandler"] = [](const std::string& message) { on_error.Invoke(message); };
  sol::protected_function::set_default_handler(state["OvisErrorHandler"]);

  sol::usertype<vector2> vector2_type =
      state.new_usertype<vector2>("Vector2", sol::constructors<vector2(), vector2(float, float)>());
  vector2_type["x"] = &vector2::x;
  vector2_type["y"] = &vector2::y;

  sol::usertype<vector3> vector3_type =
      state.new_usertype<vector3>("Vector3", sol::constructors<vector3(), vector3(float, float, float)>());
  vector3_type["x"] = &vector3::x;
  vector3_type["y"] = &vector3::y;
  vector3_type["z"] = &vector3::z;

  sol::usertype<vector4> vector4_type =
      state.new_usertype<vector4>("Vector4", sol::constructors<vector4(), vector4(float, float, float, float)>());
  vector4_type["x"] = &vector4::x;
  vector4_type["y"] = &vector4::y;
  vector4_type["z"] = &vector4::z;
  vector4_type["w"] = &vector4::w;

  sol::usertype<Scene> scene_type = state.new_usertype<Scene>("Scene");
  scene_type["CreateObject"] = static_cast<SceneObject* (Scene::*)(const std::string&)>(&Scene::CreateObject);
  scene_type["DeleteObject"] = &Scene::DeleteObject;
  scene_type["GetObject"] = &Scene::GetObject;

  Module::RegisterToLua();
  SceneObject::RegisterToLua();
}

sol::protected_function_result Lua::AddSceneController(const std::string& code, const std::string& id) {
  if (Module::IsSceneControllerRegistered(id)) {
    Module::DeregisterSceneController(id);
  }
  Module::RegisterSceneController(id, [id](Scene*) { return std::make_unique<ScriptSceneController>(id); });
  return state.do_string(code, "=" + id);
}

// bool LoadScript(ResourceManager* resource_manager, const json& parameters, const std::string& id,
//                 const std::string& directory) {
//   if (!parameters.contains("type")) {
//     LogE("Script has no type!");
//     return false;
//   }
//   if (!parameters.contains("type")) {
//     LogE("Script has no type!");
//     return false;
//   }

//   const std::string type = parameters["type"];
//   if (type == "scene_controller") {
//     const std::string source_filename = directory + '/' + id + ".lua";
//     state.script_file(source_filename);
//     auto source_code = LoadTextFile(source_filename);
//     if (!source_code) {
//       LogE("Cannot open source file: '{}'", source_filename);
//       return false;
//     } else {
//       state.script(*source_code);
//       const std::string controller_id = id.substr(0, id.find('.'));
//       Module::RegisterSceneController(
//           controller_id, [controller_id](Scene*) { return std::make_unique<ScriptSceneController>(controller_id); });
//       return true;
//     }
//   } else {
//     LogE("Invalid script type: '{}'", type);
//     return false;
//   }
// }

}  // namespace ovis
