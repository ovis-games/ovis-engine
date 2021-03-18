#include <sol/sol.hpp>

#include <ovis/core/core_module.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/math/math_module.hpp>
#include <ovis/scene/scene.hpp>
#include <ovis/scene/scene_controller.hpp>
#include <ovis/scene/scene_object.hpp>
#include <ovis/scene/scene_object_component.hpp>
#include <ovis/scene/transform.hpp>
#include <ovis/scene/transform_controller.hpp>

namespace ovis {

int LuaCoreModule(lua_State* l) {
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

  return core_module.push();
}

int LoadSceneModule(lua_State* l) {
  sol::state_view state(l);

  /// Engine module
  // @usage local engine = require('ovis.engine')
  sol::table scene_module = state.create_table();

  Scene::RegisterType(&scene_module);
  SceneObject::RegisterType(&scene_module);
  SceneObjectComponent::RegisterType(&scene_module);
  Transform::RegisterType(&scene_module);
  SceneController::RegisterType(&scene_module);
  TransformController::RegisterType(&scene_module);

  return scene_module.push();
}

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