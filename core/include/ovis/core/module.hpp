#pragma once

#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/core/function.hpp>
#include <ovis/core/type.hpp>

namespace ovis {

class Type;
class Function;

class Module : public std::enable_shared_from_this<Module> {
 public:
  static std::shared_ptr<Module> Register(std::string_view name);
  static void Deregister(std::string_view name);
  static std::shared_ptr<Module> Get(std::string_view name);
  static std::span<std::shared_ptr<Module>> registered_modules() { return modules; }

  ~Module();

  std::string_view name() const { return name_; }

  // Types
  std::shared_ptr<Type> RegisterType(TypeDescription description);
  std::shared_ptr<Type> GetType(std::string_view name);
  // std::span<std::shared_ptr<Type>> types() { return types_; }
  // std::span<const std::shared_ptr<Type>> types() const { return types_; }

  // Functions
  std::shared_ptr<Function> RegisterFunction(FunctionDescription description);
  std::shared_ptr<Function> GetFunction(std::string_view name);
  std::span<std::shared_ptr<Function>> functions() { return functions_; }
  std::span<const std::shared_ptr<Function>> functions() const { return functions_; }

  json Serialize() const;

 private:
  Module(std::string_view name) : name_(name) {}

  std::string name_;
  std::vector<TypeId> types_;
  std::vector<std::shared_ptr<Function>> functions_;

  static std::vector<std::shared_ptr<Module>> modules;
};

// Implementation
inline std::shared_ptr<Module> Module::Get(std::string_view name) {
  for (const auto& module : modules) {
    if (module->name() == name) {
      return module;
    }
  }
  return nullptr;
}

inline std::shared_ptr<Type> Module::GetType(std::string_view name) {
  for (const auto& type_id : types_) {
    if (const auto type = Type::Get(type_id); type && type->name() == name) {
      return type;
    }
  }
  return nullptr;
}

inline std::shared_ptr<Function> Module::GetFunction(std::string_view name) {
  return nullptr;
}

}  // namespace ovis
