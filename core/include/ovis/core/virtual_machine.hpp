#pragma once

#include <any>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <vector>

#include <ovis/utils/down_cast.hpp>
#include <ovis/utils/json.hpp>
#include <ovis/utils/parameter_pack.hpp>
#include <ovis/utils/range.hpp>
#include <ovis/utils/result.hpp>
#include <ovis/utils/safe_pointer.hpp>
#include <ovis/utils/type_id.hpp>
#include <ovis/utils/versioned_index.hpp>
#include <ovis/core/value_storage.hpp>
#include <ovis/core/virtual_machine_instructions.hpp>

namespace ovis {

// Forward declarations
class Type;
class Value;
class Function;
class Module;
class ExecutionContext;

using NativeFunction = Result<>(ExecutionContext*);

template <typename T> constexpr bool is_reference_type_v = std::is_base_of_v<SafelyReferenceable, T>;
template <typename T> constexpr bool is_pointer_to_reference_type_v = std::is_pointer_v<T> && std::is_base_of_v<SafelyReferenceable, std::remove_pointer_t<T>>;
template <typename T> constexpr bool is_value_type_v = !std::is_base_of_v<SafelyReferenceable, T> && !std::is_pointer_v<T> && !std::is_same_v<std::remove_cvref_t<T>, Value>;
template <typename T> constexpr bool is_pointer_to_value_type_v = std::is_pointer_v<T> && is_value_type_v<std::remove_pointer_t<T>>;

template <typename T> concept ReferenceType = is_reference_type_v<T>;
template <typename T> concept PointerToReferenceType = is_pointer_to_reference_type_v<T>;
template <typename T> concept ValueType = is_value_type_v<T>;
template <typename T> concept PointerToValueType = is_pointer_to_value_type_v<T>;


namespace detail {
template <typename T> struct TypeWrapper { using type = T; };
template <typename T> struct TypeWrapper<T&> { using type = std::reference_wrapper<T>; };
template <typename... T> struct TypeWrapper<std::tuple<T...>> { using type = std::tuple<typename TypeWrapper<T>::type...>; };
}
template <typename T> using WrappedType = typename detail::TypeWrapper<T>::type;

class VirtualMachine {
};

class ExecutionContext {
 public:
  ExecutionContext(std::size_t register_count = 1024);

  ValueStorage& top(std::size_t offset = 0);

  void PushValue() { return PushValues(1); }
  void PushValues(std::size_t count);
  template <typename T> void PushValue(T&& value);
  void PopTrivialValue() { PopTrivialValues(1); }
  void PopTrivialValues(std::size_t count);
  void PopValue() { PopValues(1); }
  void PopValues(std::size_t count);
  void PopAll() { PopValues(used_register_count_); }

  void PushStackFrame();
  void PopStackFrame();

  Value& GetValue(std::size_t position, std::size_t stack_frame_offset = 0);

  // Returns an arbitrary value from the stack. In case T is a tuple it will be filled with the values at positions
  // [position, position + tuple_size_v<T>).
  template <typename T> WrappedType<T> GetValue(std::size_t position, std::size_t stack_frame_offset = 0);

  Value& GetTopValue(std::size_t offset_from_top = 0);
  // Returns an arbitrary value from the top of the stack. In case T is a tuple it will be filled with the values at
  // offsets: (offset_from_top + tuple_size_v<T>, offset_from_top).
  template <typename T> WrappedType<T> GetTopValue(std::size_t offset_from_top = 0);

  std::span<const ValueStorage> registers() const { return { registers_.get(), used_register_count_}; }
  std::span<const ValueStorage> current_function_scope_registers() const { return { registers_.get(), used_register_count_}; }
  Result<> Execute(std::span<const vm::Instruction> instructions, std::span<const ValueStorage> constants);

  static ExecutionContext* global_context() { return &global; }

 private:
  struct StackFrame {
    std::size_t register_offset;
  };

  std::unique_ptr<ValueStorage[]> registers_;
  std::size_t register_count_;
  std::size_t used_register_count_;
  // Every exuction frame always has the base stack frame that cannot be popped
  // which simplifies the code
  std::vector<StackFrame> stack_frames_;

  static ExecutionContext global;
};

// Implementation
inline ValueStorage& ExecutionContext::top(std::size_t offset) {
  assert(offset < used_register_count_);
  return registers_[used_register_count_ - (offset + 1)];
}

inline void ExecutionContext::PushValues(std::size_t count) {
  // if (used_register_count_ + count > register_count_) {
  //   return Error("Stack overflow");
  // }
  // for (auto i : IRange(count)) {
  //   assert(registers_[used_register_count_ + i].native_type_id_ == TypeOf<void>);
  // }
  used_register_count_ += count;
  // return Success;
}


template <typename T>
inline void ExecutionContext::PushValue(T&& value) {
  // OVIS_CHECK_RESULT(PushValue());
  PushValue();
  top().reset(std::forward<T>(value));
  // return Success;
}

// template <typename T>
// inline void ExecutionContext::PushValueView(T&& value) {
//   detail::ValueHelper<T>::PushValueView(this, std::forward<T>(value));
// }

inline void ExecutionContext::PopTrivialValues(std::size_t count) {
  assert(count <= used_register_count_);
  for (auto i : IRange(count)) {
    registers_[used_register_count_ - (i + 1)].reset_trivial();
  }
  used_register_count_ -= count;
}

inline void ExecutionContext::PopValues(std::size_t count) {
  assert(count <= used_register_count_);
  for (auto i : IRange(count)) {
    registers_[used_register_count_ - (i + 1)].reset();
  }
  used_register_count_ -= count;
}

}  // namespace ovis
