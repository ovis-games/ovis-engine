#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <ovis/core/log.hpp>
#include <ovis/engine/module.hpp>
#include <ovis/engine/scene_object_component.hpp>

namespace ovis {

std::vector<std::string> SceneObjectComponent::GetRegisteredComponents() {
  std::vector<std::string> component_ids;
  component_ids.reserve(Module::scene_object_component_factory_functions()->size());
  for (const auto& component_factory : *Module::scene_object_component_factory_functions()) {
    component_ids.push_back(component_factory.first);
  }
  return component_ids;
}

}  // namespace ovis
