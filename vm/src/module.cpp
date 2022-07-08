#include <ovis/vm/module.hpp>

namespace ovis {

Module::~Module() {
  for (const auto type_id : types_) {
    virtual_machine()->DeregisterType(type_id);
  }
}

Type* Module::RegisterType(TypeDescription description) {
  description.module = this;
  return virtual_machine()->RegisterType(std::move(description));
}

Type* Module::GetType(std::string_view name) {
  for (const auto& type_id : types_) {
    if (const auto type = virtual_machine()->GetType(type_id); type && type->name() == name) {
      return type;
    }
  }
  return nullptr;
}

std::shared_ptr<Function> Module::RegisterFunction(FunctionDescription description) {
  functions_.push_back(std::make_shared<Function>(description));
  return functions_.back();
}

std::shared_ptr<Function> Module::GetFunction(std::string_view name) {
  for (const auto& function : functions_) {
    if (function->name() == name) {
      return function;
    }
  }

  return nullptr;
}

void Module::AddType(TypeId type_id) {
  assert(std::find(types_.begin(), types_.end(), type_id) == types_.end());
  types_.push_back(type_id);
}

void Module::RemoveType(TypeId type_id) {
  assert(std::find(types_.begin(), types_.end(), type_id) != types_.end());
  types_.erase(std::find(types_.begin(), types_.end(), type_id));
}

}  // namespace ovis

