namespace ovis {
namespace vm {

inline ExecutionContext::ExecutionContext(std::size_t reserved_stack_size) {
  stack_.reserve(reserved_stack_size);
  PushStackFrame();
}

namespace detail {

template <std::size_t N, typename... Types>
using nth_parameter_t = std::tuple_element_t<N, std::tuple<Types...>>;
static_assert(std::is_same_v<void, nth_parameter_t<0, void, bool, int>>);
static_assert(std::is_same_v<bool, nth_parameter_t<1, void, bool, int>>);
static_assert(std::is_same_v<int, nth_parameter_t<2, void, bool, int>>);

template <typename T>
struct ValueHelper {
  static void Push(ExecutionContext* context, T&& value) {
    context->PushValue(Value(std::forward<T>(value)));
  }

  static WrappedType<T> Get(ExecutionContext* context, std::size_t position, std::size_t stack_frame_offset) {
    return context->GetValue(position, stack_frame_offset).Get<T>();
  }

  static WrappedType<T> GetTop(ExecutionContext* context, std::size_t offset_from_top) {
    return context->GetTopValue(offset_from_top).Get<T>();
  }
};

template <>
struct ValueHelper<Value> {
  static void Push(ExecutionContext* context, Value value) {
    context->PushValue(std::move(value));
  }

  static Value Get(ExecutionContext* context, std::size_t position, std::size_t stack_frame_offset) {
    assert(false);
    return context->GetValue(position, stack_frame_offset);
  }

  static Value GetTop(ExecutionContext* context, std::size_t offset_from_top) {
    return context->GetTopValue(offset_from_top);
  }
};

template <typename... T>
struct ValueHelper<std::tuple<T...>> {
  template <std::size_t... I>
  static void PushTupleImpl(ExecutionContext* context, std::tuple<T...>&& values, std::index_sequence<I...>) {
    ((context->PushValue(std::get<I>(values))), ...);
  }
  template <typename Indices = std::make_index_sequence<sizeof...(T)>>
  static void PushTuple(ExecutionContext* context, std::tuple<T...>&& values) {
    PushTupleImpl(context, std::forward<std::tuple<T...>>(values), Indices{});
  }
  static void Push(ExecutionContext* context, std::tuple<T...>&& value) {
    PushTuple(context, std::forward<std::tuple<T...>>(value));
  }

  template <std::size_t... I>
  static auto GetTupleValueImpl(ExecutionContext* context, std::size_t base_position,
                                            std::size_t stack_frame_offset, std::index_sequence<I...>) {
    return std::tuple<WrappedType<T>...>(context->GetValue<nth_parameter_t<I, T...>>(base_position + I, stack_frame_offset)...);
  }
  template <typename Indices = std::make_index_sequence<sizeof...(T)>>
  static auto GetTupleValue(ExecutionContext* context, std::size_t base_position, std::size_t stack_frame_offset) {
    return GetTupleValueImpl(context, base_position, stack_frame_offset, Indices{});
  }

  static auto Get(ExecutionContext* context, std::size_t position, std::size_t stack_frame_offset) {
    return GetTupleValue(context, position, stack_frame_offset);
  }

  template <std::size_t... I>
  static std::tuple<T...> GetTopTupleImpl(ExecutionContext* context, std::size_t offset_from_top, std::index_sequence<I...>) {
    return std::tuple<WrappedType<T>...>(context->GetTopValue(offset_from_top + sizeof...(I) - I - 1).Get<nth_parameter_t<I, T...>>()...);
  }
  template <typename Indices = std::make_index_sequence<sizeof...(T)>>
  static std::tuple<T...> GetTopTuple(ExecutionContext* context, std::size_t offset_from_top) {
    return GetTopTupleImpl(context, offset_from_top, Indices{});
  }
  static std::tuple<WrappedType<T>...> GetTop(ExecutionContext* context, std::size_t offset_from_top) {
    return GetTopTuple(context, offset_from_top);
  }
};

}

template <typename T>
inline void ExecutionContext::PushValue(T&& value) {
  if constexpr (std::is_same_v<std::remove_reference_t<T>, Value>) {
    stack_.push_back(value);
  } else {
    detail::ValueHelper<T>::Push(this, std::forward<T>(value));
  }
}

inline void ExecutionContext::PopValue() {
  assert(stack_frames_.back().base_position < stack_.size());
  stack_.pop_back();
}

inline void ExecutionContext::PopValues(std::size_t count) {
  assert(stack_frames_.size() > 1);
  assert(count <= stack_.size());
  assert(stack_frames_.back().base_position <= stack_.size() - count);
  stack_.erase(stack_.end() - count, stack_.end());
}

inline void ExecutionContext::PushStackFrame() {
  stack_frames_.push_back({
      .base_position = stack_.size()
  });
}

inline void ExecutionContext::PopStackFrame() {
  assert(stack_frames_.size() > 1);
  stack_.resize(stack_frames_.back().base_position);
  stack_frames_.pop_back();
}

inline Value& ExecutionContext::GetValue(std::size_t position, std::size_t stack_frame_offset) {
  assert(stack_frames_.size() > 0);
  assert(stack_frame_offset < stack_frames_.size());
  const auto& stack_frame = *(stack_frames_.rbegin() + stack_frame_offset);
  assert(stack_frame.base_position + position < stack_.size());
  
  return stack_[stack_frame.base_position + position];
}

template <typename T>
inline WrappedType<T> ExecutionContext::GetValue(std::size_t position, std::size_t stack_frame_offset) {
  return detail::ValueHelper<T>::Get(this, position, stack_frame_offset);
}

inline Value& ExecutionContext::GetTopValue(std::size_t offset_from_top) {
  assert(stack_frames_.size() > 0);
  assert(offset_from_top < stack_.size());
  assert(offset_from_top < stack_.size() - stack_frames_.back().base_position);
  return *(stack_.rbegin() + offset_from_top);
}

template <typename T>
inline WrappedType<T> ExecutionContext::GetTopValue(std::size_t offset_from_top) {
  return detail::ValueHelper<T>::GetTop(this, offset_from_top);
}

inline std::optional<std::ptrdiff_t> ExecutionContext::operator()(const instructions::NativeFunctionCall& native_function_call) {
  native_function_call.function_pointer(this);
  return 1;
}

inline std::optional<std::ptrdiff_t> ExecutionContext::operator()(const instructions::PushConstant& push_constant) {
  stack_.push_back(push_constant.value);
  return 1;
}

inline std::optional<std::ptrdiff_t> ExecutionContext::operator()(const instructions::PushStackValue& push_stack_value) {
  PushValue(GetValue(push_stack_value.position, push_stack_value.stack_frame_offset));
  return 1;
}

inline std::optional<std::ptrdiff_t> ExecutionContext::operator()(const instructions::Assign& assign_stack_value) {
  GetValue(assign_stack_value.position, assign_stack_value.stack_frame_offset) = GetTopValue(0);
  PopValue();
  return 1;
}

inline std::optional<std::ptrdiff_t> ExecutionContext::operator()(const instructions::Pop& pop) {
  PopValue();
  return 1;
}

inline std::optional<std::ptrdiff_t> ExecutionContext::operator()(const instructions::Jump& jump) {
  return jump.instruction_offset;
}

inline std::optional<std::ptrdiff_t> ExecutionContext::operator()(const instructions::JumpIfTrue& jump) {
  const bool condition = GetTopValue<bool>();
  PopValue();
  return condition ? jump.instruction_offset : 1;
}

inline std::optional<std::ptrdiff_t> ExecutionContext::operator()(const instructions::JumpIfFalse& jump) {
  const bool condition = GetTopValue<bool>();
  PopValue();
  return condition ? 1 : jump.instruction_offset;
}

inline std::optional<std::ptrdiff_t> ExecutionContext::operator()(const instructions::PushStackFrame& push_scope) {
  PushStackFrame();
  return 1;
}

inline std::optional<std::ptrdiff_t> ExecutionContext::operator()(const instructions::PopStackFrame& pop_scope) {
  PopStackFrame();
  return 1;
}

inline std::optional<std::ptrdiff_t> ExecutionContext::operator()(const instructions::Return&) {
  return {};
}

}
}
