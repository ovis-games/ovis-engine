#pragma once

#include <cstdlib>
#include <any>
#include <cassert>
#include <cstdint>
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

namespace ovis {
namespace vm {

// Forward declarations
class Type;
class Value;
class Function;
class Module;
class ExecutionContext;

using NativeFunction = Result<>(ExecutionContext*);
// using NativeFunction* = Result<>(*)(ExecutionContext*);

class Type : public std::enable_shared_from_this<Type> {
  friend class Value;
  friend class Module;

 public:
  using Id = VersionedIndex<uint32_t, 20>;
  static constexpr Id NONE_ID = Id(0);

  // using DefaultConstructFunction = Result<>(void* value);
  // using DestroyFunction = Result<>(void* value);
  // using CopyFunction = Result<>(void* destination, const void* source);

  using ConversionFunction = Value(*)(Value& value);
  using SerializeFunction = json(*)(const Value& value);
  using DeserializeFunction = Value(*)(const json& value);

  struct Property {
    using GetFunction = std::add_pointer_t<Value(const Value& object)>;
    using SetFunction = std::add_pointer_t<void(Value* object, const Value& property_value)>;

    Id type_id;
    std::string name;
    GetFunction getter;
    SetFunction setter;
  };

  std::string_view name() const { return name_; }
  std::string_view full_reference() const { return full_reference_; }
  std::weak_ptr<Type> parent() const { return parent_; }
  std::weak_ptr<Module> module() const { return module_; }

  // bool IsDerivedFrom(std::shared_ptr<Type> type) const;
  // template <typename T> bool IsDerivedFrom() const;

  // void RegisterConstructorFunction(std::shared_ptr<Function> function);
  // template <typename... Args> Value Construct(Args&&... args) const;

  // void SetSerializeFunction(SerializeFunction function) { serialize_function_ = function; }
  // SerializeFunction serialize_function() const { return serialize_function_; }

  // void SetDeserializeFunction(DeserializeFunction function) { deserialize_function_ = function; }
  // DeserializeFunction deserialize_function() const { return deserialize_function_; }

  // Value CreateValue(const json& data) const;

  // void RegisterProperty(std::string_view name, Id type_id, Property::GetFunction getter,
  //                       Property::SetFunction setter = nullptr);
  // template <auto PROPERTY> requires std::is_member_pointer_v<decltype(PROPERTY)>
  // void RegisterProperty(std::string_view);

  // template <auto GETTER>
  // void RegisterProperty(std::string_view);

  // template <auto GETTER, auto SETTER>
  // void RegisterProperty(std::string_view);

  // const Property* GetProperty(std::string_view name) const;
  // std::span<const Property> properties() const { return properties_; }

  static Id Register(std::shared_ptr<Type> type, TypeId native_type_id = TypeOf<void>);
  static Result<> Deregister(Id id);
  template <typename T> static std::shared_ptr<Type> Get();
  template <typename T> static std::optional<Id> GetId();
  static std::shared_ptr<Type> Get(Id id);

  static std::shared_ptr<Type> Deserialize(const json& data);

  // json Serialize() const;

 private:
  Type(std::shared_ptr<Module> module, std::string_view name);
  Type(std::shared_ptr<Module> module, std::string_view name, std::shared_ptr<Type> parent, ConversionFunction from_base, ConversionFunction to_base);
  template <typename T, typename ParentType> static std::shared_ptr<Type> Create(std::shared_ptr<Module> module, std::string_view name);

  std::vector<Property> properties_;

  std::string name_;
  std::string full_reference_;
  std::weak_ptr<Type> parent_;
  std::weak_ptr<Module> module_;

  std::size_t alignment_in_bytes_;
  std::size_t size_in_bytes_;
  NativeFunction* default_construct_;
  NativeFunction* copy_construct_;
  NativeFunction* assign_;
  NativeFunction* destruct_;

  ConversionFunction from_base_;
  ConversionFunction to_base_;
  SerializeFunction serialize_function_;
  DeserializeFunction deserialize_function_;
  std::vector<std::weak_ptr<Function>> constructor_functions_;

  struct Registration {
    Id vm_type_id;
    TypeId native_type_id;
    std::shared_ptr<Type> type;
  };

  // static std::vector<std::pair<TypeId, std::weak_ptr<Type>>> type_associations;
  static std::vector<Registration> registered_types;
};

template <typename T> constexpr bool is_reference_type_v = std::is_base_of_v<SafelyReferenceable, T>;
template <typename T> constexpr bool is_pointer_to_reference_type_v = std::is_pointer_v<T> && std::is_base_of_v<SafelyReferenceable, std::remove_pointer_t<T>>;
template <typename T> constexpr bool is_value_type_v = !std::is_base_of_v<SafelyReferenceable, T> && !std::is_pointer_v<T> && !std::is_same_v<std::remove_cvref_t<T>, Value>;
template <typename T> constexpr bool is_pointer_to_value_type_v = std::is_pointer_v<T> && is_value_type_v<std::remove_pointer_t<T>>;

template <typename T> concept ReferenceType = is_reference_type_v<T>;
template <typename T> concept PointerToReferenceType = is_pointer_to_reference_type_v<T>;
template <typename T> concept ValueType = is_value_type_v<T>;
template <typename T> concept PointerToValueType = is_pointer_to_value_type_v<T>;

class Value {
 public:
  const void* data() const { return &data_; }
  void* data() { return &data_; }

  template <typename T> T& as() {
    assert(TypeOf<T> == native_type_id_);
    return *reinterpret_cast<T*>(data());
  }

  template <typename T> const T& as() const {
    assert(TypeOf<T> == native_type_id_);
    return *reinterpret_cast<const T*>(data());
  }

 private:
   std::aligned_storage_t<8, 8> data_;
#ifndef NDEBUG
   Type::Id vm_type_id_;
   TypeId native_type_id_;
#endif
};

namespace detail {

template <typename T>
void Destruct(void* object) {
  reinterpret_cast<T*>(object)->~T();
}

}  // namespace detail

class alignas(16) ValueStorage final {
 public:
  using DestructFunction = void(void*);

  constexpr static std::size_t ALIGNMENT = 8;
  constexpr static std::size_t SIZE = 8;

  // We will fit pointers in the value field, so they should fit and properly aligned
  static_assert(alignof(void*) <= ALIGNMENT);
  static_assert(sizeof(void*) <= SIZE);

  ValueStorage() : destruct_function_and_flags(0) {}
  template <typename T>
  ValueStorage(T&& value) : ValueStorage() {
    set(std::forward<T>(value));
  }
  ValueStorage(const ValueStorage& other) = delete;
  ValueStorage(ValueStorage&& other) = delete;
  ~ValueStorage() { reset(); }

  ValueStorage& operator=(const ValueStorage& other) = delete;
  ValueStorage& operator=(ValueStorage&& other) = delete;

  template <typename T>
  void set(T&& value) {
    reset();

    using StoredType = std::remove_reference_t<T>;
    if constexpr (alignof(T) > ALIGNMENT || sizeof(T) > SIZE) {
      allocate(alignof(T), sizeof(T));
      SetAllocatedStorageFlag(true);
      new (data_as_pointer()) StoredType(std::forward<T>(value));
    } else {
      new (data()) StoredType(std::forward<T>(value));
    }
    if constexpr (!std::is_trivially_destructible_v<T>) {
      SetDestructFunction(&detail::Destruct<StoredType>);
    }
#ifndef NDEBUG
    native_type_id_ = TypeOf<StoredType>;
#endif
  }

  void reset_trivial() {
    assert(!allocated_storage());
    assert(destruct_function() == nullptr);
#ifndef NDEBUG
    native_type_id_ = TypeOf<void>;
#endif
  }

  void reset() {
    const bool is_storage_allocated = allocated_storage();
    DestructFunction* destruct = destruct_function();
    if (destruct) {
      destruct(is_storage_allocated ? data_as_pointer() : data());
      set_destruct_function(nullptr);
    }
    if (is_storage_allocated) {
      deallocate();
      SetAllocatedStorageFlag(false);
    }
#ifndef NDEBUG
    native_type_id_ = TypeOf<void>;
#endif
  }

  void allocate(std::size_t alignment, std::size_t size) {
    new (&data_) void*(aligned_alloc(alignment, size));
    SetAllocatedStorageFlag(true);
  }

  void deallocate() {
    assert(allocated_storage());
    std::free(*reinterpret_cast<void**>(&data_));
    SetAllocatedStorageFlag(false);
  }

  bool allocated_storage() const { return flags_.allocated_storage != 0; }

  void set_destruct_function(DestructFunction* destructor) { SetDestructFunction(destructor); }

  DestructFunction* destruct_function() const {
    return reinterpret_cast<DestructFunction*>(destruct_function_and_flags & ~1);
  }

  const void* data() const { return &data_; }
  void* data() { return &data_; }

  const void* data_as_pointer() const {
    assert(allocated_storage());
    return *reinterpret_cast<void* const*>(&data_);
  }
  void* data_as_pointer() {
    assert(allocated_storage());
    return *reinterpret_cast<void* const*>(&data_);
  }

  template <typename T>
  T& as() {
    assert(TypeOf<T> == native_type_id_);
    if constexpr (alignof(T) > ALIGNMENT || sizeof(T) > SIZE) {
      return *reinterpret_cast<T*>(data_as_pointer());
    } else {
      return *reinterpret_cast<T*>(data());
    }
  }

  template <typename T>
  const T& as() const {
    assert(TypeOf<T> == native_type_id_);
    if constexpr (alignof(T) > ALIGNMENT || sizeof(T) > SIZE) {
      return *reinterpret_cast<const T*>(data_as_pointer());
    } else {
      return *reinterpret_cast<const T*>(data());
    }
  }

  static void copy_trivially(ValueStorage* destination, const ValueStorage* source) {
    assert(!destination->allocated_storage());
    assert(!source->allocated_storage());
    std::memcpy(&destination->data_, &source->data_, SIZE);
    destination->destruct_function_and_flags = source->destruct_function_and_flags;
#ifndef NDEBUG
    destination->native_type_id_ = source->native_type_id_;
#endif
  }

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

  void SetDestructFunction(DestructFunction* function) {
    const std::uintptr_t pointer_as_int = reinterpret_cast<std::uintptr_t>(function);
    assert((pointer_as_int & 1) == 0);
    destruct_function_and_flags = pointer_as_int | flags_.allocated_storage;
  }

  void SetAllocatedStorageFlag(bool value) {
    // See https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
    destruct_function_and_flags ^= (-static_cast<std::uintptr_t>(value) ^ destruct_function_and_flags) & 1;
  }

#ifndef NDEBUG
 public:
  TypeId native_type_id_ = TypeOf<void>;
#endif
};

#ifdef NDEBUG
static_assert(sizeof(ValueStorage) == 16);
#endif

enum class OpCode : std::uint32_t {
  EXIT,
  PUSH,
  POP,
  POP_TRIVIAL,
  COPY_TRIVIAL_VALUE,
  PUSH_TRIVIAL_CONSTANT,
  CALL_NATIVE_FUNCTION,
  SUBTRACT_NUMBERS,
  MULTIPLY_NUMBERS,
  IS_NUMBER_GREATER,
  JUMP,
  JUMP_IF_TRUE,
  JUMP_IF_FALSE,
};

namespace instructions {

constexpr std::size_t OPCODE_BITS = 8;
constexpr std::size_t TYPE_ALIGN_BITS = 3;
constexpr std::size_t TYPE_SIZE_BITS = 11;
constexpr std::size_t REGISTER_INDEX_BITS = 12;
constexpr std::size_t CONSTANT_INDEX_BITS = 10;
constexpr std::size_t JUMP_OFFSET_BITS = 24;

struct ConstructValue {
  std::uint32_t opcode : OPCODE_BITS;
};
static_assert(sizeof(ConstructValue) == sizeof(std::uint32_t));

struct ConstructLargeValue {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t align : TYPE_ALIGN_BITS;
  std::uint32_t size : TYPE_SIZE_BITS;
};
static_assert(sizeof(ConstructLargeValue) == sizeof(std::uint32_t));

struct DestructValue {
  std::uint32_t opcode : OPCODE_BITS;
};
static_assert(sizeof(DestructValue) == sizeof(std::uint32_t));

struct DestructLargeValue {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t align : TYPE_ALIGN_BITS;
  std::uint32_t size : TYPE_SIZE_BITS;
};
static_assert(sizeof(DestructLargeValue) == sizeof(std::uint32_t));

struct CopyValue {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t destination : REGISTER_INDEX_BITS;
  std::uint32_t source : REGISTER_INDEX_BITS;
};
static_assert(sizeof(CopyValue) == sizeof(std::uint32_t));

struct CopyLargeValue {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t destination : REGISTER_INDEX_BITS;
  std::uint32_t source : REGISTER_INDEX_BITS;
};
static_assert(sizeof(CopyLargeValue) == sizeof(std::uint32_t));

struct CopyTrivialValueData {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t destination : REGISTER_INDEX_BITS;
  std::uint32_t source : REGISTER_INDEX_BITS;
};
static_assert(sizeof(CopyTrivialValueData) == sizeof(std::uint32_t));

struct PushConstant {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t constant : CONSTANT_INDEX_BITS;
};
static_assert(sizeof(PushConstant) == sizeof(std::uint32_t));

struct PushTrivialConstantData {
  std::uint32_t opcode : OPCODE_BITS = static_cast<std::uint32_t>(OpCode::PUSH_TRIVIAL_CONSTANT);
  std::uint32_t constant : CONSTANT_INDEX_BITS;
};
static_assert(sizeof(PushTrivialConstantData) == sizeof(std::uint32_t));

struct PushLargeConstant {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t constant : CONSTANT_INDEX_BITS;
  std::uint32_t align : TYPE_ALIGN_BITS;
  std::uint32_t size : TYPE_SIZE_BITS;
};
static_assert(sizeof(PushLargeConstant) == sizeof(std::uint32_t));

struct PushPopData {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t count : 8;
};
static_assert(sizeof(PushPopData) == sizeof(std::uint32_t));

struct CallNativeFunctionData {
  std::uint32_t opcode : OPCODE_BITS = static_cast<std::uint32_t>(OpCode::CALL_NATIVE_FUNCTION);
  std::uint32_t input_count : 8;
};
static_assert(sizeof(CallNativeFunctionData ) == sizeof(std::uint32_t));

struct JumpData {
  std::uint32_t opcode : OPCODE_BITS;
  std::int32_t offset : JUMP_OFFSET_BITS;
};
static_assert(sizeof(JumpData) == sizeof(std::uint32_t));

struct NumberOperationData {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t result : 8;
  std::uint32_t first : 8;
  std::uint32_t second : 8;
};
static_assert(sizeof(NumberOperationData ) == sizeof(std::uint32_t));

}  // namespace instructions

union Instruction {
  std::uint32_t opcode : instructions::OPCODE_BITS;

  instructions::PushPopData push_pop;
  instructions::PushTrivialConstantData push_trivial_constant;
  instructions::CallNativeFunctionData call_native_function;
  instructions::JumpData jump_data;
  instructions::CopyTrivialValueData copy_trivial_value;
  instructions::NumberOperationData number_operation_data;
};

namespace instructions {

inline Instruction Exit() {
  return { .opcode = static_cast<std::uint32_t>(OpCode::EXIT) };
}

inline Instruction Push(std::uint32_t count) {
  return {.push_pop = {.opcode = static_cast<std::uint32_t>(OpCode::PUSH), .count = count}};
}

inline Instruction Pop(std::uint32_t count) {
  return {.push_pop = {.opcode = static_cast<std::uint32_t>(OpCode::POP), .count = count}};
}

inline Instruction PopTrivial(std::uint32_t count) {
  return {.push_pop = {.opcode = static_cast<std::uint32_t>(OpCode::POP_TRIVIAL), .count = count}};
}

inline Instruction CopyTrivialValue(std::uint32_t destination, std::uint32_t source) {
  return {.copy_trivial_value = {.opcode = static_cast<std::uint32_t>(OpCode::COPY_TRIVIAL_VALUE),
                                 .destination = destination,
                                 .source = source}};
}

inline Instruction PushTrivialConstant(std::uint32_t constant_index) {
  return {.push_trivial_constant = {.constant = constant_index}};
}

inline Instruction CallNativeFunction(std::uint32_t input_count) {
  return {.call_native_function = { .input_count = input_count }};
}

inline Instruction Jump(std::int32_t offset) {
  return {.jump_data = {.opcode = static_cast<std::uint32_t>(OpCode::JUMP), .offset = offset}};
}

inline Instruction JumpIfTrue(std::int32_t offset) {
  return {.jump_data = {.opcode = static_cast<std::uint32_t>(OpCode::JUMP_IF_TRUE), .offset = offset}};
}

inline Instruction JumpIfFalse(std::int32_t offset) {
  return {.jump_data = {.opcode = static_cast<std::uint32_t>(OpCode::JUMP_IF_FALSE), .offset = offset}};
}

inline Instruction SubtractNumbers(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = static_cast<std::uint32_t>(OpCode::SUBTRACT_NUMBERS),
      .result = result,
      .first = first,
      .second = second
    }
  };
}

inline Instruction MultiplyNumbers(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = static_cast<std::uint32_t>(OpCode::MULTIPLY_NUMBERS),
      .result = result,
      .first = first,
      .second = second
    }
  };
}

inline Instruction IsNumberGreater(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = static_cast<std::uint32_t>(OpCode::IS_NUMBER_GREATER),
      .result = result,
      .first = first,
      .second = second
    }
  };
}

}  // namespace instructions

namespace detail {
template <typename T> struct TypeWrapper { using type = T; };
template <typename T> struct TypeWrapper<T&> { using type = std::reference_wrapper<T>; };
template <typename... T> struct TypeWrapper<std::tuple<T...>> { using type = std::tuple<typename TypeWrapper<T>::type...>; };
}
template <typename T> using WrappedType = typename detail::TypeWrapper<T>::type;

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
  Result<> Execute(std::span<const Instruction> instructions, std::span<const ValueStorage> constants);

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

class Module : public std::enable_shared_from_this<Module> {
 public:
  static std::shared_ptr<Module> Register(std::string_view name);
  static void Deregister(std::string_view name);
  static std::shared_ptr<Module> Get(std::string_view name);
  static std::span<std::shared_ptr<Module>> registered_modules() { return modules; }

  ~Module();

  std::string_view name() const { return name_; }

  // Types
  std::shared_ptr<Type> RegisterType(std::string_view name);
  std::shared_ptr<Type> RegisterType(std::string_view name, std::shared_ptr<Type> parent_type,
                                     Type::ConversionFunction from_base, Type::ConversionFunction to_base);

  template <typename T, typename ParentType = void>
  std::shared_ptr<Type> RegisterType(std::string_view name, bool create_cpp_association = true);

  std::shared_ptr<Type> GetType(std::string_view name);
  // std::span<std::shared_ptr<Type>> types() { return types_; }
  // std::span<const std::shared_ptr<Type>> types() const { return types_; }

  // Functions
  std::shared_ptr<Function> RegisterFunction(std::string_view name, NativeFunction* function_pointer,
                                             std::vector<Function::ValueDeclaration> inputs,
                                             std::vector<Function::ValueDeclaration> outputs);

  template <auto FUNCTION>
  std::shared_ptr<Function> RegisterFunction(std::string_view name, std::vector<std::string_view> input_names,
                                             std::vector<std::string_view> output_names);

  std::shared_ptr<Function> GetFunction(std::string_view name);
  std::span<std::shared_ptr<Function>> functions() { return functions_; }
  std::span<const std::shared_ptr<Function>> functions() const { return functions_; }

  json Serialize() const;

 private:
  Module(std::string_view name) : name_(name) {}

  std::string name_;
  std::vector<Type::Id> types_;
  std::vector<std::shared_ptr<Function>> functions_;

  static std::vector<std::shared_ptr<Module>> modules;
};
}  // namespace vm
}  // namespace ovis

#include <ovis/core/virtual_machine_type.inl>
#include <ovis/core/virtual_machine_value.inl>
#include <ovis/core/virtual_machine_function.inl>
#include <ovis/core/virtual_machine_execution_context.inl>
#include <ovis/core/virtual_machine_module.inl>
