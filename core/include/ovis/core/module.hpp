#pragma once

#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/core/function.hpp>
#include <ovis/core/type.hpp>

namespace ovis {

class VirtualMachine;
class Type;
class Function;

class Module : public std::enable_shared_from_this<Module> {
 public:
  ~Module();

  std::string_view name() const { return name_; }
  VirtualMachine* virtual_machine() const { return virtual_machine_; }

  // Types
  Type* RegisterType(TypeDescription description);
  Type* GetType(std::string_view name);
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

  VirtualMachine* virtual_machine_;
  std::string name_;
  std::vector<TypeId> types_;
  std::vector<std::shared_ptr<Function>> functions_;
};


}  // namespace ovis
