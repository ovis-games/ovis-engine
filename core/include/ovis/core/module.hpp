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
  std::shared_ptr<Type> RegisterType(std::string_view name);
  std::shared_ptr<Type> RegisterType(std::string_view name, std::shared_ptr<Type> parent_type, NativeFunction from_base,
                                     NativeFunction to_base);

  template <typename T, typename ParentType = void>
  std::shared_ptr<Type> RegisterType(std::string_view name, bool create_cpp_association = true);

  std::shared_ptr<Type> GetType(std::string_view name);
  // std::span<std::shared_ptr<Type>> types() { return types_; }
  // std::span<const std::shared_ptr<Type>> types() const { return types_; }

  // Functions
  std::shared_ptr<Function> RegisterFunction(std::string_view name, NativeFunction* function_pointer,
                                             std::vector<Function::ValueDeclaration> inputs,
                                             std::vector<Function::ValueDeclaration> outputs);

  template <auto FUNCTION>
  std::shared_ptr<Function> RegisterFunction(std::string_view name, std::vector<std::string_view> input_names,
                                             std::vector<std::string_view> output_names);

  std::shared_ptr<Function> GetFunction(std::string_view name);
  std::span<std::shared_ptr<Function>> functions() { return functions_; }
  std::span<const std::shared_ptr<Function>> functions() const { return functions_; }

  json Serialize() const;

 private:
  Module(std::string_view name) : name_(name) {}

  std::string name_;
  std::vector<Type::Id> types_;
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

template <typename T, typename ParentType>
inline std::shared_ptr<Type> Module::RegisterType(std::string_view name, bool create_cpp_association) {
  if (Type::Get<T>() != nullptr) {
    return nullptr;
  }

  if (create_cpp_association && Type::Get<T>() != nullptr) {
    return nullptr;
  }

  if (GetType(name) != nullptr) {
    return nullptr;
  }

  std::shared_ptr<Type> type = Type::Create<T, ParentType>(shared_from_this(), name);
  types_.push_back(create_cpp_association ? Type::Register(type, TypeOf<T>) : Type::Register(type));
  return type;
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
