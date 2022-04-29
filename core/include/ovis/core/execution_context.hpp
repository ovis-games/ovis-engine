#pragma once

#include <vector>
#include <span>

#include <ovis/utils/result.hpp>
#include <ovis/utils/not_null.hpp>
#include <ovis/core/function_handle.hpp>
#include <ovis/core/virtual_machine_instructions.hpp>

namespace ovis {

class ValueStorage;
class VirtualMachine;

class ExecutionContext {
 public:
  ExecutionContext(NotNull<VirtualMachine*> virtual_machine, std::size_t register_count = 1024);

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

  void PushStackFrame();
  void PopStackFrame();

  std::span<const ValueStorage> registers() const;
  std::span<const ValueStorage> current_function_scope_registers() const;
  Result<> Execute(std::span<const Instruction> instructions, std::span<const ValueStorage> constants);

  template <typename ReturnType, typename... ArgumentTypes>
  Result<ReturnType> Call(FunctionHandle handle, ArgumentTypes&&... arguments);

 private:
  struct StackFrame {
    std::size_t register_offset;
  };

  NotNull<VirtualMachine*> virtual_machine_;
  std::unique_ptr<ValueStorage[]> registers_;
  std::size_t register_count_;
  std::size_t used_register_count_;
  // Every exuction frame always has the base stack frame that cannot be popped
  // which simplifies the code
  std::vector<StackFrame> stack_frames_;
};

template <auto FUNCTION>
Result<> NativeFunctionWrapper(ExecutionContext* context);

}  // namespace ovis

#include <ovis/utils/parameter_pack.hpp>
#include <ovis/utils/reflection.hpp>
#include <ovis/core/value_storage.hpp>

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
  top().reset(std::forward<T>(value));
  // return Success;
}

template <typename... Ts>
void ExecutionContext::PushValues(Ts&&... value) {
  detail::PushValues<Ts...>::Push(this, std::forward<Ts>(value)...);
}

template <typename ReturnType, typename... ArgumentTypes>
inline Result<ReturnType> ExecutionContext::Call(FunctionHandle handle, ArgumentTypes&&... arguments) {
  // Reserve space for return value if necessary
  if constexpr (!std::is_same_v<ReturnType, void>) {
    PushUninitializedValue();
  }
  // Push arguments
  PushValues(std::forward<ArgumentTypes>(arguments)...);

  Result<> result = Success;
  if (handle.is_script_function) {
    assert(false && "Not implemented yet");
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
