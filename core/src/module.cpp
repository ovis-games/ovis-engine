#include <ovis/core/module.hpp>

namespace ovis {

// std::vector<std::shared_ptr<Module>> Module::modules;

// std::shared_ptr<Module> Module::Register(std::string_view name) {
//   if (Get(name) != nullptr) {
//     return nullptr;
//   }

//   modules.push_back(std::shared_ptr<Module>(new Module(name)));
//   return modules.back();
// }

// void Module::Deregister(std::string_view name) {
//   std::erase_if(modules, [name](const std::shared_ptr<Module>& module) {
//     return module->name() == name;
//   });
// }

Module::~Module() {
  for (const auto type_id : types_) {
    virtual_machine()->RemoveType(type_id);
  }
}

Type* Module::RegisterType(TypeDescription description) {
  auto type = virtual_machine()->AddType(shared_from_this(), std::move(description));
  if (type) {
    types_.push_back(type->id());
  }
  return type.get();
}

Type* Module::GetType(std::string_view name) {
  for (const auto& type_id : types_) {
    if (const auto type = virtual_machine()->GetType(type_id); type && type->name() == name) {
      return type;
    }
  }
  return nullptr;
}

inline std::shared_ptr<Function> Module::GetFunction(std::string_view name) {
  return nullptr;
}


}  // namespace ovis

