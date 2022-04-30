#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/utils/native_type_id.hpp>
#include <ovis/utils/range.hpp>
#include <ovis/vm/execution_context.hpp>
#include <ovis/vm/type_id.hpp>
#include <ovis/vm/virtual_machine_instructions.hpp>

namespace ovis {

// Forward declarations
class Type;
class Value;
class ValueStorage;
class Module;
struct TypeDescription;

class VirtualMachine {
  friend class Module;

 public:
  VirtualMachine(std::size_t constants_capacity = 1024, std::size_t instruction_capacity = 1024 * 1024);

  ExecutionContext* main_execution_context() { return &main_execution_context_; }

  // Registers a new function with the specified instructions and constants
  std::size_t RegisterFunction(std::span<const Instruction> instructions, std::span<const Value> constants);
  const Instruction* GetInstructionPointer(std::size_t offset) const;
  std::span<ValueStorage> GetConstantRange(std::size_t offset, std::size_t count);

  std::shared_ptr<Module> RegisterModule(std::string_view name);
  void DeregisterModule(std::string_view name);
  std::shared_ptr<Module> GetModule(std::string_view name);
  auto registered_modules() {
    return TransformRange(registered_modules_, [](const auto& module) { return module.get(); });
  }

  template <typename T, typename ParentType = void> Type* RegisterType(std::string_view name, Module* module = nullptr);
  Type* RegisterType(TypeDescription description);

  Result<> DeregisterType(TypeId type_id);
  Result<> DeregisterType(NotNull<Type*> type);

  template <typename T> TypeId GetTypeId();
  TypeId GetTypeId(NativeTypeId native_type_id);
  TypeId GetTypeId(const json& json);

  template <typename T> Type* GetType();
  Type* GetType(TypeId id);
  Type* GetType(NativeTypeId id);
  Type* GetType(const json& json);

 private:
  ExecutionContext main_execution_context_;
  std::unique_ptr<ValueStorage[]> constants_;
  std::unique_ptr<Instruction[]> instructions_;

  std::vector<std::shared_ptr<Module>> registered_modules_;

  struct TypeRegistration {
    TypeId id;
    NativeTypeId native_type_id;
    std::shared_ptr<Type> type;
  };
  std::vector<TypeRegistration> registered_types_;
  TypeId FindFreeTypeId();

};

extern VirtualMachine vm;

}  // namespace ovis

#include <ovis/vm/type.hpp>
#include <ovis/vm/value_storage.hpp>

namespace ovis {

template <typename T, typename ParentType = void>
Type* RegisterType(std::string_view name, Module* module) {
  return RegisterType(TypeDescription::CreateForNativeType<T, ParentType>(name, module));
}

template <typename T>
TypeId VirtualMachine::GetTypeId() {
  auto type_id = GetTypeId(TypeOf<T>);
  if constexpr (!std::is_same_v<void,T> && !std::is_same_v<void*,T>) {
    if (!registered_types_[type_id.index].type) {
      registered_types_[type_id.index].type =
          std::make_shared<Type>(type_id, TypeDescription::CreateForNativeType<T>(&vm, ""));
    }
  }
  return type_id;
}

template <typename T>
Type* VirtualMachine::GetType() {
  return registered_types_[GetTypeId<T>().index].type.get();
}

}  // namespace ovis
