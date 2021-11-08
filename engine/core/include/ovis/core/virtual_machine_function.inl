namespace ovis {
namespace vm {

template <typename... OutputTypes, typename... InputTypes>
inline FunctionResultType<OutputTypes...> Function::Call(InputTypes&&... inputs) {
  return Call<OutputTypes...>(ExecutionContext::global_context(), std::forward<InputTypes>(inputs)...);
}

template <typename... OutputTypes, typename... InputTypes>
inline FunctionResultType<OutputTypes...> Function::Call(ExecutionContext* context, InputTypes&&... inputs) {
  assert(sizeof...(InputTypes) == inputs_.size());
  // TODO: validate input/output types
  context->PushStackFrame();
  ((context->PushValue(std::forward<InputTypes>(inputs))), ...);
  function_pointer_(context);
  if constexpr (sizeof...(OutputTypes) == 0) {
    context->PopStackFrame();
  } else {
    auto result = context->GetTopValue<FunctionResultType<OutputTypes...>>();
    context->PopStackFrame();
    return result;
  }
}

inline std::optional<std::size_t> Function::GetInputIndex(std::string_view input_name) const {
  for (const auto& input : IndexRange(inputs_)) {
    if (input->name == input_name) {
      return input.index();
    }
  }
  return {};
}

inline std::optional<Function::ValueDeclaration> Function::GetInput(std::string_view input_name) const {
  for (const auto& input : inputs_) {
    if (input.name == input_name) {
      return input;
    }
  }
  return {};
}

inline std::optional<Function::ValueDeclaration> Function::GetInput(std::size_t input_index) const {
  if (input_index < inputs_.size()) {
    return inputs_[input_index];
  } else {
    return {};
  }
}

inline std::optional<std::size_t> Function::GetOutputIndex(std::string_view output_name) const {
  for (const auto& output : IndexRange(outputs_)) {
    if (output->name == output_name) {
      return output.index();
    }
  }
  return {};
}

inline std::optional<Function::ValueDeclaration> Function::GetOutput(std::string_view output_name) const {
  for (const auto& output : outputs_) {
    if (output.name == output_name) {
      return output;
    }
  }
  return {};
}

inline std::optional<Function::ValueDeclaration> Function::GetOutput(std::size_t output_index) const {
  if (output_index < outputs_.size()) {
    return outputs_[output_index];
  } else {
    return {};
  }
}

template <typename... InputTypes>
inline bool Function::IsCallableWithArguments() const {
  if (inputs_.size() != sizeof...(InputTypes)) {
    return false;
  }
  std::array<safe_ptr<Type>, sizeof...(InputTypes)> input_types = {
    Type::Get<InputTypes>()...
  };

  for (auto i : IRange(sizeof...(InputTypes))) {
    if (inputs_[i].type != input_types[i]) {
      return false;
    }
  }
  return true;
}

inline Function::Function(std::string_view name, FunctionPointer function_pointer, std::vector<ValueDeclaration> inputs,
                          std::vector<ValueDeclaration> outputs)
    : name_(name), function_pointer_(function_pointer), inputs_(inputs), outputs_(outputs) {
  text_ = name_;
  for (const auto input: inputs_) {
    text_ += fmt::format(" ({})", input.name);
  }
}

}
}
