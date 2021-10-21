#include "ovis/core/math_operations.hpp"
#include <middleclass.hpp>
#include <sol/sol.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/utils/reflection.hpp>
#include <ovis/core/camera.hpp>
#include <ovis/core/core_module.hpp>
#include <ovis/core/event.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/scene_controller.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/scene_object_component.hpp>
#include <ovis/core/transform.hpp>

namespace ovis {

namespace {

double Add(double x, double y) {
  return x + y;
}

}

bool LoadCoreModule() {
  static safe_ptr<Module> core_module;
  if (core_module == nullptr) {
    core_module = Module::Register("Core");
    core_module->RegisterType<double>("Number");
    core_module->RegisterFunction<&Add>("Add", {"x", "y"}, {"result"});

    SceneObjectComponent::Register("Transform",
                                   [](SceneObject* object) { return std::make_unique<Transform>(object); });
    SceneObjectComponent::Register("Camera", [](SceneObject* object) { return std::make_unique<Camera>(object); });


    // Vector2::RegisterType(global_script_context());
    // Vector3::RegisterType(global_script_context());
    // SceneObject::RegisterType(global_script_context());
    // Scene::RegisterType(global_script_context());
  }

  return true;
}

}  // namespace ovis
