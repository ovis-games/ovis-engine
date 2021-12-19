#include <ovis/core/function.hpp>
#include <ovis/core/module.hpp>

namespace ovis {

std::shared_ptr<Function> Function::Deserialize(const json& data) {
  if (!data.contains("module")) {
    return nullptr;
  }
  const auto& module_json = data.at("module");
  if (!module_json.is_string()) {
    return nullptr;
  }
  const std::shared_ptr<Module> module = Module::Get(module_json.get_ref<const std::string&>());
  if (module == nullptr) {
    return nullptr;
  }
 if (!data.contains("name")) {
    return nullptr;
  }
  const auto& name_json = data.at("name");
  if (!name_json.is_string()) {
    return nullptr;
  }
  return module->GetFunction(name_json.get_ref<const std::string&>());
}

}  // namespace ovis
