#include <ovis/core/module.hpp>

namespace ovis {

std::vector<std::shared_ptr<Module>> Module::modules;

std::shared_ptr<Module> Module::Register(std::string_view name) {
  if (Get(name) != nullptr) {
    return nullptr;
  }

  modules.push_back(std::shared_ptr<Module>(new Module(name)));
  return modules.back();
}

void Module::Deregister(std::string_view name) {
  std::erase_if(modules, [name](const std::shared_ptr<Module>& module) {
    return module->name() == name;
  });
}

Module::~Module() {
  for (const auto type_id : types_) {
    Type::Deregister(type_id);
  }
}

}  // namespace ovis

