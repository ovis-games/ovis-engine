#pragma once

#include <vector>
#include <span>

#include <ovis/utils/result.hpp>
#include <ovis/utils/not_null.hpp>
#include <ovis/vm/function_handle.hpp>
#include <ovis/vm/virtual_machine_instructions.hpp>

namespace ovis {

class ValueStorage;
class VirtualMachine;

// The execution context handles all function calling and instruction execution.
// It contains configurable fixed size stack.
//
// For native function the calling convention is as follows:
// [output 0, output 1, ..., output n, argument 0, argument 1, ... argument n]
// Where output 0 ... n represent "None" values which serve as placeholders for the function outputs which are filled by
// the return instruction.
//
// For script functions the calling convention is as follows:
// [output (0...n), return address, constant offset, stack offset, arguments (0...n), function address]

class ExecutionContext {
 public:
  static constexpr std::size_t DEFAULT_STACK_SIZE = 1024; // 16KB stack size

  ExecutionContext(NotNull<VirtualMachine*> virtual_machine, std::size_t register_count = DEFAULT_STACK_SIZE);

  NotNull<VirtualMachine*> virtual_machine() { return virtual_machine_; }

  ValueStorage& top(std::size_t offset = 0);

  void PushUninitializedValue() { return PushUninitializedValues(1); }
  void PushUninitializedValues(std::size_t count);
  template <typename T> void PushValue(T&& value);
  template <typename... Ts> void PushValues(Ts&&... value);
  void PopTrivialValue() { PopTrivialValues(1); }
  void PopTrivialValues(std::size_t count);
  void PopValue() { PopValues(1); }
  void PopValues(std::size_t count);
  void PopAll() { PopValues(used_register_count_); }

  template <typename T> requires(!std::is_same_v<std::remove_cv_t<T>, ValueStorage>)
  T& GetStackValue(std::size_t offset);
  ValueStorage& GetStackValue(std::size_t offset);

  std::span<const ValueStorage> registers() const;
  std::span<const ValueStorage> current_function_scope_registers() const;
  Result<> Execute(std::uintptr_t instruction_offset);
  Result<> Execute(NotNull<const Instruction*> instructions);

  template <typename ReturnType, typename... ArgumentTypes>
  Result<ReturnType> Call(FunctionHandle handle, ArgumentTypes&&... arguments);

  static constexpr std::size_t GetOutputOffset(std::size_t output_index) {
    return output_index;
  }
  static constexpr std::size_t GetReturnAddressOffset(std::size_t output_count) {
    return GetOutputOffset(output_count);
  }
  static constexpr std::size_t GetConstantOffset(std::size_t output_count) {
    return GetReturnAddressOffset(output_count) + 1;
  }
  static constexpr std::size_t GetStackOffset(std::size_t output_count) {
    return GetConstantOffset(output_count) + 1;
  }
  static constexpr std::size_t GetInputOffset(std::size_t output_count, std::size_t input_index) {
    return GetStackOffset(output_count) + 1 + input_index;
  }
  static constexpr std::size_t GetFunctionBaseOffset(std::size_t output_count, std::size_t input_count) {
    return GetInputOffset(output_count, input_count);
  }

 private:
  NotNull<VirtualMachine*> virtual_machine_;
  std::unique_ptr<ValueStorage[]> registers_;
  std::size_t register_count_;
  std::size_t used_register_count_;

  std::uint32_t stack_offset_ = 0;
  std::uint32_t constant_offset_ = 0;
};

template <auto FUNCTION>
Result<> NativeFunctionWrapper(ExecutionContext* context);

}  // namespace ovis

#include <ovis/utils/parameter_pack.hpp>
#include <ovis/utils/reflection.hpp>
#include <ovis/vm/value_storage.hpp>
// #include <ovis/vm/virtual_machine.hpp>

namespace ovis {

namespace detail {

template <typename... InputTypes>
struct PushValues;

template <typename InputType, typename... InputTypes>
struct PushValues<InputType, InputTypes...> {
  static void Push(ExecutionContext* context, InputType&& input, InputTypes&&... inputs) {
    context->PushValue(std::forward<InputType>(input));
    PushValues<InputTypes...>::Push(context, std::forward<InputTypes>(inputs)...);
  }
};

template <>
struct PushValues<> {
  static void Push(ExecutionContext* context) {}
};

template <typename... Args, std::size_t... I>
std::tuple<Args...> GetInputTuple(ExecutionContext* context, TypeList<Args...>, std::index_sequence<I...>) {
  return std::tuple<Args...>(context->top(sizeof...(Args) - I - 1).as<nth_parameter_t<I, Args...>>()...);
}

}  // namespace detail

template <typename T>
void ExecutionContext::PushValue(T&& value) {
  // OVIS_CHECK_RESULT(PushValue());
  PushUninitializedValue();
  top().Store(std::forward<T>(value));
  // return Success;
}

template <typename... Ts>
void ExecutionContext::PushValues(Ts&&... value) {
  detail::PushValues<Ts...>::Push(this, std::forward<Ts>(value)...);
}

template <typename T> requires(!std::is_same_v<std::remove_cv_t<T>, ValueStorage>)
T& ExecutionContext::GetStackValue(std::size_t offset) {
  return GetStackValue(offset).as<T>();
}

inline ValueStorage& ExecutionContext::GetStackValue(std::size_t offset) {
  return *(registers_.get() + offset);
}

template <typename ReturnType, typename... ArgumentTypes>
inline Result<ReturnType> ExecutionContext::Call(FunctionHandle handle, ArgumentTypes&&... arguments) {
  // Reserve space for return value if necessary
  if constexpr (!std::is_same_v<ReturnType, void>) {
    PushUninitializedValue();
  }

  if (handle.is_script_function) {
    PushValue(std::uint32_t(0)); // Return address (0 = exit)
    PushValue(constant_offset_); // Constant offset
    PushValue(stack_offset_); // Stack offset
  }

  // Push arguments
  PushValues(std::forward<ArgumentTypes>(arguments)...);

  Result<> result = Success;
  if (handle.is_script_function) {
    result = Execute(handle.instruction_offset);
  } else {
    result = handle.native_function(this);
  }

  if (!result) {
    return result.error();
  }

  // Function call was successful, extract return value if necessary
  if constexpr (std::is_same_v<ReturnType, void>) {
    // No return type, just succeed
    return Success;
  } else {
    ReturnType result(top().as<ReturnType>());
    PopValue();
    return result;
  }
}


template <auto FUNCTION>
Result<> NativeFunctionWrapper(ExecutionContext* context) {
  using ArgumentTypes = typename reflection::Invocable<FUNCTION>::ArgumentTypes;
  using ReturnType = typename reflection::Invocable<FUNCTION>::ReturnType;

  auto input_tuple = detail::GetInputTuple(context, ArgumentTypes{}, std::make_index_sequence<ArgumentTypes::size>{});
  context->PopValues(ArgumentTypes::size);
  if constexpr (!std::is_same_v<void, ReturnType>) {
    context->PushValue(std::apply(FUNCTION, input_tuple));
  } else {
    std::apply(FUNCTION, input_tuple);
  }
  return Success;
}

}  // namespace ovis
