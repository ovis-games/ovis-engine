#include <middleclass.hpp>
#include <sol/sol.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/core/camera.hpp>
#include <ovis/core/core_module.hpp>
#include <ovis/core/event.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/scene_controller.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/scene_object_component.hpp>
#include <ovis/core/script_scene_controller.hpp>
#include <ovis/core/transform.hpp>
#include <ovis/core/transform_controller.hpp>

namespace ovis {

int LoadCoreModule(lua_State* l) {
  sol::state_view state(l);

  /// This module provides core components of the engine.
  // @module ovis.core
  // @usage local core = require('ovis.core')
  sol::table core_module = state.create_table();

  /// Creates a class.
  // @function class
  // @tparam string name The name of the function
  // @param[opt] parent Parent class
  // @usage local SomeClass = core.class("SomeClass")
  // function SomeClass:initialize(initial_value)
  //   self.a = initial_value
  // end
  // function SomeClass:add(num)
  //   self.a = self.a + num
  //   return self.a
  // end
  // local class_instance = SomeClass:new(10)
  // result = class_instance:add(20)
  // assert(result == 30)
  core_module["class"] = state.require_script("class", middleclass::SOURCE, false);

  /// Writes a message to the output log.
  // When called with a single parameter it will be converted into a string using `tostring()`.
  // When called with more parameters, they are passed to string.format. This requires the first
  // parameter to be a formatting string and the remaining ones it's arguments.
  // @remark When you want to print a '%' in a formatted string you have to escape it like this: '%%'.
  // @function log
  // @tparam string message
  // @param[opt] formatting_arguments... Arguments that should be passed to string.format
  // @usage core.log("This is a simple message")
  // core.log(100)
  // core.log("A nicely formatted string: %d%%", 100)
  core_module["log"] = [](const sol::lua_value& message, sol::variadic_args va) {
    sol::state_view state(message.value().lua_state());

    std::string message_string;
    if (va.size() == 0) {
      message_string = state["tostring"](message);
    } else {
      sol::function f;
      sol::protected_function format = state["string"]["format"];
      sol::protected_function_result result = format(message, va);
      if (!result.valid()) {
        return;
      }
      message_string = result;
    }
    ovis::LogI("{}", message_string);
  };

  Event::RegisterType(&core_module);
  Scene::RegisterType(&core_module);
  SceneObject::RegisterType(&core_module);
  SceneObjectComponent::RegisterType(&core_module);
  Transform::RegisterType(&core_module);
  Camera::RegisterType(&core_module);
  SceneController::RegisterType(&core_module);
  ScriptSceneController::RegisterType(&core_module);
  TransformController::RegisterType(&core_module);
  Transform::RegisterType(&core_module);
  Vector2::RegisterType(&core_module);
  Vector3::RegisterType(&core_module);
  Color::RegisterType(&core_module);

  return core_module.push();
}

bool LoadCoreModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::math, sol::lib::table,
                       sol::lib::package);
    SceneObjectComponent::Register("Transform", []() { return std::make_unique<Transform>(); });
    SceneObjectComponent::Register("Camera", []() { return std::make_unique<Camera>(); });

    lua.require("ovis.core", &LoadCoreModule);
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis