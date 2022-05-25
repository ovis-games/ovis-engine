#pragma once

#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include <ovis/utils/not_null.hpp>
#include <ovis/utils/json.hpp>
#include <ovis/vm/type_id.hpp>

namespace ovis {

class VirtualMachine;
struct TypeDescription;
class Type;
struct FunctionDescription;
class Function;

class Module : public std::enable_shared_from_this<Module> {
  friend class Type;
  friend class Function;

 public:
  Module(NotNull<VirtualMachine*> virtual_machine, std::string_view name) : virtual_machine_(virtual_machine), name_(name) {}
  ~Module();

  std::string_view name() const { return name_; }
  NotNull<VirtualMachine*> virtual_machine() const { return virtual_machine_; }

  // Types
  template <typename T, typename ParentType = void>
  Type* RegisterType(std::string_view name);
  Type* RegisterType(TypeDescription description);
  Type* GetType(std::string_view name);

  // Functions
  std::shared_ptr<Function> RegisterFunction(FunctionDescription description);
  std::shared_ptr<Function> GetFunction(std::string_view name);
  std::span<std::shared_ptr<Function>> functions() { return functions_; }
  std::span<const std::shared_ptr<Function>> functions() const { return functions_; }

  json Serialize() const;

 private:
  NotNull<VirtualMachine*> virtual_machine_;
  std::string name_;
  std::vector<TypeId> types_;
  std::vector<std::shared_ptr<Function>> functions_;

  void AddType(TypeId type_id);
  void RemoveType(TypeId type_id);
};

}  // namespace ovis

#include <ovis/vm/function.hpp>
#include <ovis/vm/type.hpp>
#include <ovis/vm/virtual_machine.hpp>

namespace ovis {

template <typename T, typename ParentType>
Type* Module::RegisterType(std::string_view name) {
  return virtual_machine()->RegisterType(TypeDescription::CreateForNativeType<T, ParentType>(this, name));
}

}  // namespace ovis
