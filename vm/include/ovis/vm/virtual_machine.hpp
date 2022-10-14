#pragma once

#include <cstddef>
#include <memory>
#include <set>
#include <span>
#include <type_traits>
#include <vector>

#include "ovis/utils/json.hpp"
#include "ovis/utils/range.hpp"
#include "ovis/utils/reflection.hpp"
#include "ovis/vm/execution_context.hpp"
#include "ovis/vm/function.hpp"
#include "ovis/vm/type.hpp"
#include "ovis/vm/type_id.hpp"
#include "ovis/vm/value.hpp"
#include "ovis/vm/value_storage.hpp"

namespace ovis {

// Forward declarations
struct TypeDescription;
class Type;
class Function;
class Value;
class ValueStorage;

template <typename T>
class FunctionWrapper;

template <typename ResultType, typename... ArgumentTypes>
class FunctionWrapper<ResultType(ArgumentTypes...)> {
 public:
  FunctionWrapper(std::shared_ptr<Function> function);

  auto function() const { return function_; }

  Result<ResultType> operator()(ArgumentTypes&&... args) const;
  Result<ResultType> operator()(ExecutionContext* execution_context, ArgumentTypes&&... args) const;
    
 private:
  std::shared_ptr<Function> function_;
};

class VirtualMachine final {
 public:
  static constexpr std::size_t DEFAULT_CONSTANT_CAPACITY = 1024; // 16KB constant storage
  static constexpr std::size_t DEFAULT_INSTRUCTION_CAPACITY = 4 * 1024 * 1024; // 4MB instruction storage

  VirtualMachine(std::size_t constant_capacity = DEFAULT_CONSTANT_CAPACITY,
                 std::size_t instruction_capacity = DEFAULT_INSTRUCTION_CAPACITY,
                 std::size_t main_execution_context_stack_size = ExecutionContext::DEFAULT_STACK_SIZE);
  ~VirtualMachine();

  ExecutionContext* main_execution_context() { return &main_execution_context_; }

  // Inserts the instructions and returns the offset
  std::size_t InsertInstructions(std::span<const Instruction> instructions);
  const Instruction* GetInstructionPointer(std::size_t offset) const;

  // Inserts the constants and returns the offset
  std::size_t InsertConstants(std::span<const Value> constants);
  const ValueStorage* GetConstantPointer(std::size_t offset) const;

  Result<> RegisterModule(std::string_view name);
  Result<> DeregisterModule(std::string_view name);
  bool IsModuleRegistered(std::string_view name);
  const std::set<std::string>& registered_modules() const { return registered_modules_; }

  template <auto MEMBER_POINTER>
  TypePropertyDescription CreateTypePropertyDescription(std::string_view name);

  template <typename T>
  TypeDescription CreateTypeDescription(std::string_view name, std::string_view module);
  template <typename T> Type* RegisterType(std::string_view name, std::string_view module);
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

  template <auto FUNCTION>
  FunctionDescription CreateFunctionDescription(std::string_view name, std::string_view module,
                                                std::vector<std::string> input_names = {},
                                                std::vector<std::string> output_names = {});
  template <auto FUNCTION>
  FunctionWrapper<std::remove_pointer_t<decltype(FUNCTION)>> CreateFunction(std::string_view name = "",
                                                                            std::string_view module = "");

  template <auto FUNCTION>
  FunctionWrapper<std::remove_pointer_t<decltype(FUNCTION)>> RegisterFunction(
      std::string_view name, std::string_view module, std::vector<std::string> input_names = {},
      std::vector<std::string> output_names = {});
  Function* RegisterFunction(FunctionDescription description);

  template <typename T>
  requires (!std::is_pointer_v<T> || std::is_function_v<std::remove_cvref_t<std::remove_pointer_t<T>>>)
  Value CreateValue(T&& native_value);
  template <typename T>
  requires (std::is_pointer_v<T> && !std::is_function_v<std::remove_cvref_t<std::remove_pointer_t<T>>>)
  Value CreateValue(T&& native_value);

 private:
  ExecutionContext main_execution_context_;

  std::unique_ptr<ValueStorage[]> constants_;
  const std::size_t constant_capacity_;
  std::size_t constant_count_;

  std::unique_ptr<Instruction[]> instructions_;
  const std::size_t instruction_capacity_;
  std::size_t instruction_count_;

  std::set<std::string> registered_modules_;

  struct TypeRegistration {
    TypeId id;
    NativeTypeId native_type_id;
    std::shared_ptr<Type> type;
  };
  std::vector<TypeRegistration> registered_types_;
  TypeId FindFreeTypeId();

  std::vector<std::shared_ptr<Function>> registered_functions_;
  std::unordered_map<std::string, TypeId> registered_function_attributes;
  std::unordered_map<std::string, TypeId> registered_type_attributes;
};

template <typename ResultType, typename... ArgumentTypes>
FunctionWrapper<ResultType(ArgumentTypes...)>::FunctionWrapper(std::shared_ptr<Function> function) : function_(std::move(function)) {
  if constexpr (std::is_same_v<ResultType, void>) {
    assert(function_->outputs().size() == 0);
  } else {
    assert(function_->outputs().size() == 1);
    assert(function_->outputs()[0].type == function_->virtual_machine()->GetTypeId<ResultType>());
  }
  assert(function_->inputs().size() == sizeof...(ArgumentTypes));
  // TODO: assert function types
}

template <typename ResultType, typename... ArgumentTypes>
Result<ResultType> FunctionWrapper<ResultType(ArgumentTypes...)>::operator()(ArgumentTypes&&... args) const {
  return operator()(function_->virtual_machine()->main_execution_context(), std::forward<ArgumentTypes>(args)...);
}

template <typename ResultType, typename... ArgumentTypes>
Result<ResultType> FunctionWrapper<ResultType(ArgumentTypes...)>::operator()(ExecutionContext* execution_context,
                                                                             ArgumentTypes&&... args) const {
  return execution_context->Call<ResultType>(function_->handle(), std::forward<ArgumentTypes>(args)...);
}

namespace detail {

template <typename T>
std::shared_ptr<Function> GetConstructFunction(VirtualMachine* virtual_machine) {
  if constexpr (std::is_default_constructible_v<T>) {
    return Function::Create(virtual_machine->CreateFunctionDescription<&type_helper::DefaultConstruct<T>>("", ""));
  } else {
    return nullptr;
  }
}

template <typename T>
std::shared_ptr<Function> GetCopyFunction(VirtualMachine* virtual_machine) {
  if constexpr (std::is_copy_assignable_v<T> && !std::is_trivially_copy_assignable_v<T>) {
    return Function::Create(virtual_machine->CreateFunctionDescription<&type_helper::CopyAssign<T>>("", ""));
  } else {
    return nullptr;
  }
}

template <typename T>
std::shared_ptr<Function> GetDestructFunction(VirtualMachine* virtual_machine) {
  if constexpr (!std::is_trivially_destructible_v<T>) {
    return Function::Create(virtual_machine->CreateFunctionDescription<&type_helper::Destruct<T>>("", ""));
  } else {
    return nullptr;
  }
}

}  // namespace detail}

template <auto PROPERTY>
TypePropertyDescription VirtualMachine::CreateTypePropertyDescription(std::string_view name) {
  return {
    .name = std::string(name),
    .type = GetTypeId<typename reflection::MemberPointer<PROPERTY>::MemberType>(),
    .access = TypePropertyDescription::PrimitiveAccess {
      .offset = reflection::MemberPointer<PROPERTY>::offset
    }
  };
}

template <typename T>
TypeDescription VirtualMachine::CreateTypeDescription(std::string_view name, std::string_view module) {
  return {
    .virtual_machine = this,
    .module = std::string(module),
    .name = std::string(name),
    .memory_layout = {
      .native_type_id = TypeOf<T>,
      .is_constructible = std::is_default_constructible_v<T>,
      .is_copyable = std::is_copy_assignable_v<T>,
      .alignment_in_bytes = alignof(T),
      .size_in_bytes = sizeof(T),
      .construct = detail::GetConstructFunction<T>(this),
      .copy =  detail::GetCopyFunction<T>(this),
      .destruct = detail::GetDestructFunction<T>(this),
    }
  };
}

template <typename T>
TypeId VirtualMachine::GetTypeId() {
  auto type_id = GetTypeId(TypeOf<T>);
  if constexpr (!std::is_same_v<void,T> && !std::is_same_v<void*,T>) {
    if (!registered_types_[type_id.index].type) {
      registered_types_[type_id.index].type = std::make_shared<Type>(type_id, CreateTypeDescription<T>("", ""));
    }
  }
  return type_id;
}

template <typename T>
Type* VirtualMachine::GetType() {
  return registered_types_[GetTypeId<T>().index].type.get();
}

template <typename T>
Type* VirtualMachine::RegisterType(std::string_view name, std::string_view module) {
  return RegisterType(CreateTypeDescription<T>(name, module));
}

namespace detail {

template <typename... ArgumentTypes>
std::vector<ValueDeclaration> MakeValueDeclaration(VirtualMachine* virtual_machine, TypeList<ArgumentTypes...>, std::vector<std::string>&& names) {
  std::array<TypeId, sizeof...(ArgumentTypes)> types = { virtual_machine->GetTypeId<ArgumentTypes>()... };
  std::vector<ValueDeclaration> declarations(sizeof...(ArgumentTypes));
  for (std::size_t i = 0; i < sizeof...(ArgumentTypes); ++i) {
    declarations[i].type = types[i];
    declarations[i].name = i < names.size() ? std::move(names[i]) : std::to_string(i);
  }
  return declarations;
}

}  // namespace detail
   //
template <auto FUNCTION>
FunctionDescription VirtualMachine::CreateFunctionDescription(std::string_view name, std::string_view module,
                                              std::vector<std::string> input_names,
                                              std::vector<std::string> output_names) {
  auto input_declarations = detail::MakeValueDeclaration(
      this, typename reflection::Invocable<FUNCTION>::ArgumentTypes{}, std::move(input_names));

  std::vector<ValueDeclaration> output_declarations;
  using ReturnType = typename reflection::Invocable<FUNCTION>::ReturnType;
  if constexpr (!std::is_same_v<void, ReturnType>) {
    output_declarations.push_back(
        {.name = output_names.size() > 0 ? std::move(output_names[0]) : "0", .type = GetTypeId<ReturnType>()});
  }

  return {
    .virtual_machine = this,
    .name = std::string(name),
    .module = std::string(module),
    .inputs = input_declarations,
    .outputs = output_declarations,
    .definition = NativeFunctionDefinition {
      .function_pointer = &NativeFunctionWrapper<FUNCTION>,
    }
  };
}

template <auto FUNCTION>
FunctionWrapper<std::remove_pointer_t<decltype(FUNCTION)>> VirtualMachine::CreateFunction(std::string_view name, std::string_view module) {
  return Function::Create(CreateFunctionDescription<FUNCTION>(name, module));
}

template <auto FUNCTION>
FunctionWrapper<std::remove_pointer_t<decltype(FUNCTION)>> VirtualMachine::RegisterFunction(std::string_view name, std::string_view module,
                                                                     std::vector<std::string> input_names,
                                                                     std::vector<std::string> output_names) {
  return RegisterFunction(
      CreateFunctionDescription<FUNCTION>(name, module, std::move(input_names), std::move(output_names)));
}

template <typename T>
requires (!std::is_pointer_v<T> || std::is_function_v<std::remove_cvref_t<std::remove_pointer_t<T>>>)
Value VirtualMachine::CreateValue(T&& native_value) {
  Value value(this);
  value.type_id_ = GetTypeId<std::remove_cvref_t<T>>();
  value.storage_.Store(std::forward<T>(native_value));
  value.is_reference_ = false;
  return value;
}

template <typename T>
requires (std::is_pointer_v<T> && !std::is_function_v<std::remove_cvref_t<std::remove_pointer_t<T>>>)
Value VirtualMachine::CreateValue(T&& native_value) {
  Value value(this);
  value.type_id_ = GetTypeId<std::remove_cvref_t<std::remove_pointer_t<T>>>();
  value.storage_.Store(std::forward<T>(native_value));
  value.is_reference_ = true;
  return value;
}


}  // namespace ovis

// #include "ovis/vm/type.hpp"
// #include "ovis/vm/value_storage.hpp"

// namespace ovis {

// template <typename T, typename ParentType>
// Type* VirtualMachine::RegisterType(std::string_view name, Module* module) {
//   return RegisterType(TypeDescription::CreateForNativeType<T, ParentType>(this, name, module));
// }


// }  // namespace ovis
