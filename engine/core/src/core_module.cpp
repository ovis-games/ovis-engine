#include "ovis/core/math_operations.hpp"

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
#include <ovis/core/transform.hpp>
#include <ovis/core/virtual_machine.hpp>
#include <ovis/core/visual_script_scene_controller.hpp>

namespace ovis {

namespace {

bool CreateBoolean(bool value) {
  return value;
}
bool Not(bool value) {
  return !value;
}
bool And(bool a, bool b) {
  return a && b;
}
bool Or(bool a, bool b) {
  return a || b;
}

double CreateNumber(double value) {
  return value;
}
double Negate(double value) {
  return -value;
}
double Add(double x, double y) {
  return x + y;
}
double Subtract(double x, double y) {
  return x - y;
}
double Multiply(double x, double y) {
  return x * y;
}
double Divide(double x, double y) {
  return x / y;
}
bool IsEqual(double x, double y) {
  return x == y;
}
bool IsNotEqual(double x, double y) {
  return x != y;
}
bool IsGreater(double x, double y) {
  return x > y;
}
bool IsGreaterOrEqual(double x, double y) {
  return x >= y;
}
bool IsLess(double x, double y) {
  return x < y;
}
bool IsLessOrEqual(double x, double y) {
  return x <= y;
}

std::string CreateString(std::string value) {
  return std::move(value);
}
void LogText(std::string text) {
  LogI("{}", text);
}

}

bool LoadCoreModule() {
  static safe_ptr<vm::Module> core_module;
  if (core_module == nullptr) {
    core_module = vm::Module::Register("Core");
    assert(core_module->name() == "Core");

    auto boolean_type = core_module->RegisterType<bool>("Boolean");
    assert(boolean_type != nullptr);

    core_module->RegisterFunction<&CreateBoolean>("Create Boolean", {"value"}, {"result"});
    core_module->RegisterFunction<&Not>("Not", {"value"}, {"result"});
    core_module->RegisterFunction<&And>("And", {"a", "b"}, {"result"});
    core_module->RegisterFunction<&Or>("Or", {"a", "b"}, {"result"});

    auto number_type = core_module->RegisterType<double>("Number");
    assert(number_type != nullptr);
    assert(vm::Type::Get<double>() != nullptr);

    core_module->RegisterFunction<&CreateNumber>("Create Number", {"value"}, {"result"});
    core_module->RegisterFunction<&Negate>("Negate", {"value"}, {"result"});
    core_module->RegisterFunction<&Add>("Add", {"x", "y"}, {"result"});
    core_module->RegisterFunction<&Subtract>("Subtract", {"x", "y"}, {"result"});
    core_module->RegisterFunction<&Multiply>("Multiply", {"x", "y"}, {"result"});
    core_module->RegisterFunction<&Divide>("Divide", {"x", "y"}, {"result"});
    core_module->RegisterFunction<&IsEqual>("Is equal", {"x", "y"}, {"result"});
    core_module->RegisterFunction<&IsNotEqual>("Is not equal", {"x", "y"}, {"result"});
    core_module->RegisterFunction<&IsGreater>("Is greater", {"x", "y"}, {"result"});
    core_module->RegisterFunction<&IsGreaterOrEqual>("Is greater or equal", {"x", "y"}, {"result"});
    core_module->RegisterFunction<&IsLess>("Is less", {"x", "y"}, {"result"});
    core_module->RegisterFunction<&IsLessOrEqual>("Is less or equal", {"x", "y"}, {"result"});

    auto string_type = core_module->RegisterType<std::string>("String");
    core_module->RegisterFunction<&CreateString>("Create String", {"value"}, {"result"});
    core_module->RegisterFunction<&LogText>("Log", {"text"}, {});

    SceneObjectComponent::Register("Transform",
                                   [](SceneObject* object) { return std::make_unique<Transform>(object); });
    SceneObjectComponent::Register("Camera", [](SceneObject* object) { return std::make_unique<Camera>(object); });


    Vector2::RegisterType(core_module.get());
    // Vector3::RegisterType(global_script_context());
    // SceneObject::RegisterType(global_script_context());
    // Scene::RegisterType(global_script_context());
  }

  return true;
}

}  // namespace ovis
