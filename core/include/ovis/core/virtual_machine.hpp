#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/utils/native_type_id.hpp>
#include <ovis/utils/range.hpp>
#include <ovis/core/execution_context.hpp>
#include <ovis/core/type_id.hpp>
#include <ovis/core/virtual_machine_instructions.hpp>

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

  // Registers a new function with the specified instructions and constants
  std::size_t RegisterFunction(std::span<const Instruction> instructions, std::span<const Value> constants);
  const Instruction* GetInstructionPointer(std::size_t offset) const;
  std::span<ValueStorage> GetConstantRange(std::size_t offset, std::size_t count);

  std::shared_ptr<Module> RegisterModule(std::string_view name);
  void DeregisterModule(std::string_view name);
  std::shared_ptr<Module> GetModule(std::string_view name);
  auto registered_modules() {
    return TransformRange(modules, [](const auto& module) { return module.get(); });
  }

  template <typename T> TypeId GetTypeId();
  template <typename T> Type* GetType();
  TypeId GetTypeId(NativeTypeId native_type_id);
  Type* GetType(TypeId id);
  TypeId GetTypeId(const json& json);
  Type* GetType(const json& json);

  static VirtualMachine* main();

 private:
  ExecutionContext main_execution_context_;
  std::unique_ptr<ValueStorage[]> constants_;
  std::unique_ptr<Instruction[]> instructions_;

  struct TypeRegistration {
    TypeId id;
    NativeTypeId native_type_id;
    std::shared_ptr<Type> type;
  };
  std::vector<TypeRegistration> registered_types;

  TypeId FindFreeTypeId();
  std::shared_ptr<Type> AddType(std::shared_ptr<Module> module, TypeDescription description);
  Result<> RemoveType(TypeId id);

  std::vector<std::shared_ptr<Module>> modules;
};

extern VirtualMachine vm;

}  // namespace ovis

#include <ovis/core/type.hpp>
#include <ovis/core/value_storage.hpp>

namespace ovis {

template <typename T>
TypeId VirtualMachine::GetTypeId() {
  auto type_id = GetTypeId(TypeOf<T>);
  if (!registered_types[type_id.index].type) {
    registered_types[type_id.index].type =
        std::make_shared<Type>(type_id, nullptr, TypeDescription::CreateForNativeType<T>(&vm, ""));
  }
  return type_id;
}

}
