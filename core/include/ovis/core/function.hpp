#pragma once

#include <memory>
#include <optional>
#include <span>
#include <vector>

#include <ovis/utils/result.hpp>
#include <ovis/core/virtual_machine.hpp>

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

class Function : public std::enable_shared_from_this<Function> {
  friend class Module;

 public:
  struct ValueDeclaration {
    std::string name;
    std::weak_ptr<Type> type;
  };

  std::string_view name() const { return name_; }
  std::string_view text() const { return text_; }
  NativeFunction* pointer() const { return function_pointer_; }

  std::span<const ValueDeclaration> inputs() const { return inputs_; }
  std::optional<std::size_t> GetInputIndex(std::string_view input_name) const;
  std::optional<ValueDeclaration> GetInput(std::string_view input_name) const;

  std::optional<ValueDeclaration> GetInput(std::size_t input_index) const;
  std::span<const ValueDeclaration> outputs() const { return outputs_; }
  std::optional<std::size_t> GetOutputIndex(std::string_view output_name) const;
  std::optional<ValueDeclaration> GetOutput(std::string_view output_name) const;
  std::optional<ValueDeclaration> GetOutput(std::size_t output_index) const;

  template <typename... InputTypes> bool IsCallableWithArguments() const;

  template <typename... OutputTypes, typename... InputsTypes>
  FunctionResultType<OutputTypes...> Call(InputsTypes&&... inputs);
  template <typename... OutputTypes, typename... InputsTypes>
  FunctionResultType<OutputTypes...> Call(ExecutionContext* context, InputsTypes&&... inputs);

  json Serialize() const;
  static std::shared_ptr<Function> Deserialize(const json& data);

 private:
  Function(std::string_view name, NativeFunction* function_pointer, std::vector<ValueDeclaration> inputs,
           std::vector<ValueDeclaration> outputs);

  std::string name_;
  std::string text_;
  NativeFunction* function_pointer_;
  std::vector<ValueDeclaration> inputs_;
  std::vector<ValueDeclaration> outputs_;
};

}  // namespace ovis

