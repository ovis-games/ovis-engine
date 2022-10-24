#pragma once

#include <memory>
#include <optional>
#include <span>
#include <type_traits>
#include <variant>
#include <vector>

#include "ovis/utils/json.hpp"
#include "ovis/utils/result.hpp"
#include "ovis/vm/function_handle.hpp"
#include "ovis/vm/type_id.hpp"
#include "ovis/vm/virtual_machine_instructions.hpp"

namespace ovis {

class Value;
class VirtualMachine;

template <typename... T> struct FunctionResult { using type = std::tuple<T...>; };
template <typename T> struct FunctionResult <T> { using type = T; };
template <> struct FunctionResult <> { using type = void; };
template <typename... T> using FunctionResultType = typename FunctionResult<T...>::type;

struct ValueDeclaration {
  std::string name;
  TypeId type;
};

struct NativeFunctionDefinition {
  NativeFunction* function_pointer;
};

struct ScriptFunctionDefinition {
  std::vector<Instruction> instructions;
  std::vector<Value> constants;
};

struct FunctionDescription {
  VirtualMachine* virtual_machine;
  std::string name;
  std::string module;
  std::vector<ValueDeclaration> inputs;
  std::vector<ValueDeclaration> outputs;
  std::variant<NativeFunctionDefinition, ScriptFunctionDefinition> definition;

  std::string PrintDefinition() const;
};

// A function can either be a native (C++) function or script function.
class Function : public std::enable_shared_from_this<Function> {
 public:
  Function(FunctionDescription description);

  std::string_view name() const { return name_; }
  std::string_view text() const { return text_; }
  VirtualMachine* virtual_machine() const { return virtual_machine_; }

  // Returns the native function pointer. It is only valid to call this function if the function is actually a native
  // function. Can be checked via is_native_function().
  NativeFunction* native_function_pointer() const;
  bool is_native_function() const { return !handle_.is_script_function; }

  // Returns the offset into the instruction storage of the virtual machine. It is only valid to call this function if
  // the function is actually a script function. You can call is_script_function() to check this.
  std::uintptr_t instruction_offset() const;
  bool is_script_function() const { return handle_.is_script_function; }

  // Returns the handle of the function
  FunctionHandle handle() const { return handle_; }

  std::span<const ValueDeclaration> inputs() const { return inputs_; }
  std::optional<std::size_t> GetInputIndex(std::string_view input_name) const;
  std::optional<ValueDeclaration> GetInput(std::string_view input_name) const;
  std::optional<ValueDeclaration> GetInput(std::size_t input_index) const;

  std::span<const ValueDeclaration> outputs() const { return outputs_; }
  std::optional<std::size_t> GetOutputIndex(std::string_view output_name) const;
  std::optional<ValueDeclaration> GetOutput(std::size_t output_index) const;
  std::optional<ValueDeclaration> GetOutput(std::string_view output_name) const;

  bool IsCallableWithArguments(std::span<const TypeId> argument_types) const;
  template <typename... InputTypes> bool IsCallableWithArguments() const;

  // // Call on main ExecutionContext of the VirtualMachine
  // template <typename OutputType, typename... InputsTypes>
  // Result<OutputType> Call(InputsTypes&&... inputs) const;

  // // Call with custom ExecutionContext
  // template <typename OutputType, typename... InputsTypes>
  // Result<OutputType> Call(ExecutionContext* execution_context, InputsTypes&&... inputs) const;

  static std::shared_ptr<Function> Create(FunctionDescription description);

 private:
  VirtualMachine* virtual_machine_;
  std::string name_;
  std::string text_;
  FunctionHandle handle_; // This handle has always the unused bit set to 0.
  std::vector<ValueDeclaration> inputs_;
  std::vector<ValueDeclaration> outputs_;

  auto FindInput(std::string_view name) const {
    return std::find_if(inputs().begin(), inputs().end(), [name](const auto& value) { return value.name == name; });
  }
  auto FindOutput(std::string_view name) const {
    return std::find_if(outputs().begin(), outputs().end(), [name](const auto& value) { return value.name == name; });
  }
};

// template <typename... ArgumentTypes>
// std::vector<ValueDeclaration> MakeValueDeclaration(VirtualMachine* virtual_machine, TypeList<ArgumentTypes...>, std::vector<std::string>&& names) {
//   std::array<TypeId, sizeof...(ArgumentTypes)> types = { virtual_machine->GetTypeId<ArgumentTypes>()... };
//   std::vector<ValueDeclaration> declarations(sizeof...(ArgumentTypes));
//   for (std::size_t i = 0; i < sizeof...(ArgumentTypes); ++i) {
//     declarations[i].type = types[i];
//     declarations[i].name = i < names.size() ? std::move(names[i]) : std::to_string(i);
//   }
//   return declarations;
// }

// }  // namespace detail

// template <auto FUNCTION>
// requires(!IsNativeFunction<decltype(FUNCTION)>) FunctionDescription FunctionDescription::CreateForNativeFunction(
//     VirtualMachine* virtual_machine, std::string name, std::vector<std::string> input_names,
//     std::vector<std::string> output_names) {

//   auto input_declarations = detail::MakeValueDeclaration(
//       virtual_machine, typename reflection::Invocable<FUNCTION>::ArgumentTypes{}, std::move(input_names));

//   std::vector<ValueDeclaration> output_declarations;
//   using ReturnType = typename reflection::Invocable<FUNCTION>::ReturnType;
//   if constexpr (!std::is_same_v<void, ReturnType>) {
//     output_declarations.push_back({
//       .name = output_names.size() > 0 ? std::move(output_names[0]) : "0",
//       .type = virtual_machine->GetTypeId<ReturnType>()
//     });
//   }

//   return CreateForNativeFunction(virtual_machine,  std::move(input_declarations),
//                                  std::move(output_declarations), std::move(name));
// }

inline std::optional<std::size_t> Function::GetInputIndex(std::string_view input_name) const {
  const auto input = FindInput(input_name);
  if (input == inputs().end()) {
    return std::nullopt;
  } else {
    return std::distance(inputs().begin(), input);
  }
}

inline std::optional<ValueDeclaration> Function::GetInput(std::string_view input_name) const {
  const auto input = FindInput(input_name);
  if (input == inputs().end()) {
    return std::nullopt;
  } else {
    return *input;
  }
}

inline std::optional<ValueDeclaration> Function::GetInput(std::size_t input_index) const {
  assert(input_index < inputs().size());
  return *(inputs().begin() + input_index);
}

inline std::optional<std::size_t> Function::GetOutputIndex(std::string_view output_name) const {
  const auto output = FindOutput(output_name);
  if (output == outputs().end()) {
    return std::nullopt;
  } else {
    return std::distance(outputs().begin(), output);
  }
}

inline std::optional<ValueDeclaration> Function::GetOutput(std::string_view output_name) const {
  const auto output = FindOutput(output_name);
  if (output == outputs().end()) {
    return std::nullopt;
  } else {
    return *output;
  }
}

inline std::optional<ValueDeclaration> Function::GetOutput(std::size_t output_index) const {
  assert(output_index < outputs().size());
  return *(outputs().begin() + output_index);
}

inline NativeFunction* Function::native_function_pointer() const {
  assert(!handle_.is_script_function);
  assert(handle_.zero == 0);
  return handle_.native_function;
}

inline std::uintptr_t Function::instruction_offset() const {
  assert(handle_.is_script_function);
  return handle_.instruction_offset;
}

// template <typename... ArgumentTypes>
// bool Function::IsCallableWithArguments() const {
//   std::array<TypeId, sizeof...(ArgumentTypes)> argument_type_ids{
//       GetTypeIdFromNativeTypeId(TypeOf<ArgumentTypes>, virtual_machine())...};
//   return IsCallableWithArguments(argument_type_ids);
// }

}  // namespace ovis

