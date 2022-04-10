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
#include <ovis/core/function_handle.hpp>
#include <ovis/core/value_storage.hpp>
#include <ovis/core/virtual_machine_instructions.hpp>

namespace ovis {

// Forward declarations
class Type;
class Value;
class Function;
class Module;
class ExecutionContext;
class ValueStorage;

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

namespace vm {

// Allocates count instructions in the vm. The offset from the beginning is returned. A span of the allocated
// instructions can be returned via GetInstructionRange(AllocateInstructions(count), count).
std::size_t AllocateInstructions(std::size_t count);
std::span<Instruction> GetInstructionRange(std::size_t offset, std::size_t count);

std::size_t AllocateConstants(std::size_t count);
std::span<ValueStorage> GetConstantRange(std::size_t offset, std::size_t count);

}  // namespace vm

class ExecutionContext {
 public:
  ExecutionContext(std::size_t register_count = 1024);

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

  template <typename ReturnType, typename... ArgumentTypes>
  Result<ReturnType> Call(FunctionHandle handle, ArgumentTypes&&... arguments);

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

class alignas(16) ValueStorage final {
 public:
  constexpr static std::size_t ALIGNMENT = 8;
  constexpr static std::size_t SIZE = 8;

  static constexpr bool IsTypeStoredInline(std::size_t alignment, std::size_t size) {
    return alignment <= ALIGNMENT && size <= SIZE;
  }

  template <typename T>
  static constexpr bool stored_inline = alignof(T) <= ALIGNMENT && sizeof(T) <= SIZE;

  // We will fit pointers and doubles in the value field, so they should fit and properly aligned
  static_assert(stored_inline<void*>);
  static_assert(stored_inline<double>);

  // Constructs an empty value storage
  ValueStorage() : destruct_function_and_flags(0) {}

  // Stores a value of type T in the storage. See reset(T&&)
  template <typename T> ValueStorage(T&& value) : ValueStorage() { reset(std::forward<T>(value)); }

  // Values cannot be moved or copied
  ValueStorage(const ValueStorage& other) = delete;
  ValueStorage(ValueStorage&& other) = delete;
  ValueStorage& operator=(const ValueStorage& other) = delete;
  ValueStorage& operator=(ValueStorage&& other) = delete;

  // The destructor will call the destructor and clean up any allocated storage
  ~ValueStorage() { reset(); }

  // Reset the storage to a new value. This will allocate storage if necessary and set the cleanup function automatically.
  template <typename T> void reset(T&& value);
  // Call this version if you know the currently stored value is not dynamically allocated and trivially destructible.
  template <typename T> void reset_trivial(T&& value);

  // Reset the storeate without a new value
  void reset();
  // Call this version if you know the currently stored value is not dynamically allocated and trivially destructible.
  void reset_trivial();


  // Sets up dynamically allocated storage
  void Allocate(std::size_t alignment, std::size_t size);

  // Deallocates allocated storage. Only call this if Allocate() was called before and the value has already been constructed.
  void Deallocate();

  // Returns true if the storage was dynamically allocated.
  bool has_allocated_storage() const { return flags_.allocated_storage != 0; }

  // Returns the pointer to the allocated storage.
  const void* allocated_storage_pointer() const {
    assert(has_allocated_storage());
    return *reinterpret_cast<void* const*>(&data_);
  }

  // Returns the pointer to the allocated storage.
  void* allocated_storage_pointer() {
    assert(has_allocated_storage());
    return *reinterpret_cast<void* const*>(&data_);
  }

  // Sets the desctuction function for the value.
  void SetDestructFunction(FunctionHandle destructor);
  // Returns the desctruction function.
  FunctionHandle destruct_function() const;

  // Returns a pointer to the internal data. Storage was allocated via Allocate() it will contain the pointer to the storage.
  const void* data() const { return &data_; }
  void* data() { return &data_; }

  const void* value_pointer() const { return has_allocated_storage() ? allocated_storage_pointer() : data(); }
  void* value_pointer() { return has_allocated_storage() ? allocated_storage_pointer() : data(); }

  // Returns the value as a reference to T. THIS METHOD WILL NOT CHECK FOR THE ALLOCATED STORAGE BIT! It assumes that
  // the value is stored internally if possible based on size and alignment.
  template <typename T> T& as();
  template <typename T> const T& as() const;

  // Trivially copies the value from source to destination. Neither the source nor the destination should have allocated storage.
  static void CopyTrivially(ValueStorage* destination, const ValueStorage* source);

 private:
  std::aligned_storage_t<SIZE, ALIGNMENT> data_;
  union {
    std::uintptr_t destruct_function_and_flags;
    // TODO: the following only works on little endian!
    struct {
      std::uintptr_t allocated_storage : 1;
      std::uintptr_t : (sizeof(std::uintptr_t) * 8 - 1);
    } flags_;
  };

  void SetAllocatedStorageFlag(bool value) {
    flags_.allocated_storage = value;
  }

#ifndef NDEBUG
 public:
  TypeId native_type_id_ = TypeOf<void>;
#endif
};

#ifdef NDEBUG
static_assert(sizeof(ValueStorage) == 16);
#endif

// Implementation
inline ValueStorage& ExecutionContext::top(std::size_t offset) {
  assert(offset < used_register_count_);
  return registers_[used_register_count_ - (offset + 1)];
}

// Implementation
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

}  // namespace detail

inline void ExecutionContext::PushUninitializedValues(std::size_t count) {
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
  PushUninitializedValue();
  top().reset(std::forward<T>(value));
  // return Success;
}

template <typename... Ts>
inline void ExecutionContext::PushValues(Ts&&... value) {
  detail::PushValues<Ts...>::Push(this, std::forward<Ts>(value)...);
}

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
    Result<ReturnType> result(top().as<ReturnType>());
    PopValue();
    return result;
  }
}

// ValueStorage
namespace detail {

template <typename T>
void Destruct(void* object) {
  reinterpret_cast<T*>(object)->~T();
}

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

template <typename T>
inline void ValueStorage::reset(T&& value) {
  reset();

  using StoredType = std::remove_reference_t<T>;
  if constexpr (alignof(T) > ALIGNMENT || sizeof(T) > SIZE) {
    Allocate(alignof(T), sizeof(T));
    SetAllocatedStorageFlag(true);
    new (allocated_storage_pointer()) StoredType(std::forward<T>(value));
  } else {
    new (data()) StoredType(std::forward<T>(value));
  }
  if constexpr (!std::is_trivially_destructible_v<StoredType>) {
    SetDestructFunction(FunctionHandle::FromNativeFunction(&NativeFunctionWrapper<detail::Destruct<StoredType>>));
  }
#ifndef NDEBUG
  native_type_id_ = TypeOf<StoredType>;
#endif
}

inline void ValueStorage::reset() {
  const bool is_storage_allocated = has_allocated_storage();
  auto destruct = destruct_function();
  if (destruct) {
    if (ExecutionContext::global_context()->Call<void>(destruct, value_pointer())) {
      SetDestructFunction(FunctionHandle::Null());
    } else {
      throw std::runtime_error("Failed to destruct object");
    }
  }
  if (is_storage_allocated) {
    Deallocate();
    SetAllocatedStorageFlag(false);
  }
#ifndef NDEBUG
  native_type_id_ = TypeOf<void>;
#endif
}

inline void ValueStorage::reset_trivial() {
  assert(!has_allocated_storage());
  assert(!destruct_function());
#ifndef NDEBUG
  native_type_id_ = TypeOf<void>;
#endif
}

inline void ValueStorage::Allocate(std::size_t alignment, std::size_t size) {
  new (&data_) void*(aligned_alloc(alignment, size));
  SetAllocatedStorageFlag(true);
}

inline void ValueStorage::Deallocate() {
  assert(has_allocated_storage());
  std::free(allocated_storage_pointer());
  SetAllocatedStorageFlag(false);
}

inline void ValueStorage::SetDestructFunction(FunctionHandle destructor) {
  assert((destructor.integer & 1) == 0);
  destruct_function_and_flags = destructor.integer | flags_.allocated_storage;
}

inline FunctionHandle ValueStorage::destruct_function() const {
  return { .integer = destruct_function_and_flags & ~1 };
}

template <typename T>
inline T& ValueStorage::as() {
  // assert(TypeOf<T> == native_type_id_ ||
  //        (!allocated_storage() && destruct_function() == nullptr && std::is_trivially_constructible_v<T>));
#ifndef NDEBUG
  native_type_id_ = TypeOf<T>;
#endif
  if constexpr (stored_inline<T>) {
    return *reinterpret_cast<T*>(data());
  } else {
    return *reinterpret_cast<T*>(allocated_storage_pointer());
  }
}

template <typename T>
inline const T& ValueStorage::as() const {
  assert(TypeOf<T> == native_type_id_);
  if constexpr (stored_inline<T>) {
    return *reinterpret_cast<const T*>(data());
  } else {
    return *reinterpret_cast<const T*>(allocated_storage_pointer());
  }
}

inline void ValueStorage::CopyTrivially(ValueStorage* destination, const ValueStorage* source) {
  assert(!destination->has_allocated_storage());
  assert(!source->has_allocated_storage());
  std::memcpy(&destination->data_, &source->data_, SIZE);
  destination->destruct_function_and_flags = source->destruct_function_and_flags;
#ifndef NDEBUG
  destination->native_type_id_ = source->native_type_id_;
#endif
}

}  // namespace ovis
