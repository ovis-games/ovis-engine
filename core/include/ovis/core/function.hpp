#pragma once

#include <memory>
#include <optional>
#include <span>
#include <vector>

#include <ovis/utils/result.hpp>
#include <ovis/core/virtual_machine.hpp>
#include <ovis/core/function_handle.hpp>

namespace ovis {

template <typename... T> struct FunctionResult { using type = std::tuple<T...>; };
template <typename T> struct FunctionResult <T> { using type = T; };
template <> struct FunctionResult <> { using type = void; };
template <typename... T> using FunctionResultType = typename FunctionResult<T...>::type;

namespace detail {

template <typename T> struct NativeFunctionWrapper;

template <typename R, typename... Args>
struct NativeFunctionWrapper<R(*)(Args...)> {
  template <R(*FUNCTION)(Args...)>
  static Result<> Call(ExecutionContext* context) {
    const auto input_tuple = GetInputTuple(context, std::make_index_sequence<sizeof...(Args)>{});
    context->PopValues(sizeof...(Args));
    if constexpr (!std::is_same_v<void, R>) {
      context->PushValue(std::apply(FUNCTION, input_tuple));
    } else {
      std::apply(FUNCTION, input_tuple);
    }
    return Success;
  }

  template <std::size_t... I>
  static std::tuple<Args...> GetInputTuple(ExecutionContext* context, std::index_sequence<I...>) {
    return std::tuple<Args...>(
      context->top(sizeof...(Args) - I - 1).as<nth_parameter_t<I, Args...>>()...
    );
  }
};

}

template <auto FUNCTION>
Result<> NativeFunctionWrapper(ExecutionContext* context) {
  return detail::NativeFunctionWrapper<decltype(FUNCTION)>::template Call<FUNCTION>(context);
}

// A function can either be a native (C++) function or script function.
class Function : public std::enable_shared_from_this<Function> {
  friend class Module;
  friend class Type;

 public:
  struct ValueDeclaration {
    std::string name;
    std::weak_ptr<Type> type;
  };

  std::string_view name() const { return name_; }
  std::string_view text() const { return text_; }

  // Returns the native function pointer. It is only valid to call this function if the function is actually a native
  // function. Can be checked via is_native_function().
  NativeFunction* native_function_pointer() const;
  bool is_native_function() const { return !handle_.is_script_function; }

  // Returns the offset into the instruction storage of the virtual machine. It is only valid to call this function if
  // the function is actually a script function. You can call is_script_function() to check this.
  std::uintptr_t instruction_offset() const;
  bool is_script_function() const { return handle_.is_script_function; }

  std::span<const ValueDeclaration> inputs() const { return inputs_; }
  std::optional<std::size_t> GetInputIndex(std::string_view input_name) const;
  std::optional<ValueDeclaration> GetInput(std::string_view input_name) const;

  std::optional<ValueDeclaration> GetInput(std::size_t input_index) const;
  std::span<const ValueDeclaration> outputs() const { return outputs_; }
  std::optional<std::size_t> GetOutputIndex(std::string_view output_name) const;
  std::optional<ValueDeclaration> GetOutput(std::string_view output_name) const;
  std::optional<ValueDeclaration> GetOutput(std::size_t output_index) const;

  template <typename... InputTypes> bool IsCallableWithArguments() const;

  // template <typename... OutputTypes, typename... InputsTypes>
  // FunctionResultType<OutputTypes...> Call(InputsTypes&&... inputs);
  // template <typename... OutputTypes, typename... InputsTypes>
  // FunctionResultType<OutputTypes...> Call(ExecutionContext* context, InputsTypes&&... inputs);
  template <typename... InputsTypes> Result<> Call(InputsTypes&&... inputs) const;

  json Serialize() const;
  static std::shared_ptr<Function> Deserialize(const json& data);

  static std::shared_ptr<Function> MakeNative(NativeFunction* function_pointer, std::vector<ValueDeclaration> inputs,
                                              std::vector<ValueDeclaration> outputs);

 private:
  Function(std::string_view name, NativeFunction* function_pointer, std::vector<ValueDeclaration> inputs,
           std::vector<ValueDeclaration> outputs);

  std::string name_;
  std::string text_;
  FunctionHandle handle_; // This handle has always the unused bit set to 0.
  std::vector<ValueDeclaration> inputs_;
  std::vector<ValueDeclaration> outputs_;
};

// Implementation
namespace detail {

template <typename... InputTypes>
struct PushValues;

template <typename InputType, typename... InputTypes>
struct PushValues<InputType, InputTypes...> {
  static void Push(InputType&& input, InputTypes&&... inputs) {
    ExecutionContext::global_context()->PushValue(std::forward<InputType>(input));
    PushValues<InputTypes...>::Push(std::forward<InputTypes>(inputs)...);
  }
};

template <>
struct PushValues<> {
  static void Push() {}
};

}  // namespace detail

template <typename... InputTypes>
inline Result<> Function::Call(InputTypes&&... inputs) const {
  detail::PushValues<InputTypes...>::Push(std::forward<InputTypes>(inputs)...);
  if (handle_.is_script_function) {
    assert(false && "Not implemented yet");
  } else {
    return native_function_pointer()(ExecutionContext::global_context());
  }
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

}  // namespace ovis

