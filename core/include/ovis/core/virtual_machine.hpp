#pragma once

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
#include <ovis/utils/range.hpp>
#include <ovis/utils/safe_pointer.hpp>

namespace ovis {
namespace vm {

// Forward declarations
class Type;
class Value;
class Function;
class Module;
class ExecutionContext;

class Type : public std::enable_shared_from_this<Type> {
  friend class Value;
  friend class Module;

 public:
  using ConversionFunction = Value(*)(Value& value);
  using SerializeFunction = json(*)(const Value& value);
  using DeserializeFunction = Value(*)(const json& value);

  struct Property {
    using GetFunction = std::add_pointer_t<Value(const Value& object)>;
    using SetFunction = std::add_pointer_t<void(Value* object, const Value& property_value)>;

    std::weak_ptr<Type> type;
    std::string name;
    GetFunction getter;
    SetFunction setter;
  };

  std::string_view name() const { return name_; }
  std::string_view full_reference() const { return full_reference_; }
  std::weak_ptr<Type> parent() const { return parent_; }
  std::weak_ptr<Module> module() const { return module_; }

  bool IsDerivedFrom(std::shared_ptr<Type> type) const;
  template <typename T> bool IsDerivedFrom() const;

  void RegisterConstructorFunction(std::shared_ptr<Function> function);
  template <typename... Args> Value Construct(Args&&... args) const;

  void SetSerializeFunction(SerializeFunction function) { serialize_function_ = function; }
  SerializeFunction serialize_function() const { return serialize_function_; }

  void SetDeserializeFunction(DeserializeFunction function) { deserialize_function_ = function; }
  DeserializeFunction deserialize_function() const { return deserialize_function_; }

  Value CreateValue(const json& data) const;

  void RegisterProperty(std::string_view name, std::shared_ptr<Type>, Property::GetFunction getter,
                        Property::SetFunction setter = nullptr);
  template <auto PROPERTY> requires std::is_member_pointer_v<decltype(PROPERTY)>
  void RegisterProperty(std::string_view);

  template <auto GETTER>
  void RegisterProperty(std::string_view);

  template <auto GETTER, auto SETTER>
  void RegisterProperty(std::string_view);

  const Property* GetProperty(std::string_view name) const;
  std::span<const Property> properties() const { return properties_; }

  template <typename T>
  static std::shared_ptr<Type> Get();

  static std::shared_ptr<Type> Deserialize(const json& data);

  json Serialize() const;

 private:
  Type(std::shared_ptr<Module> module, std::string_view name);
  Type(std::shared_ptr<Module> module, std::string_view name, std::shared_ptr<Type> parent, ConversionFunction from_base, ConversionFunction to_base);

  std::vector<Property> properties_;

  std::string name_;
  std::string full_reference_;
  std::weak_ptr<Type> parent_;
  std::weak_ptr<Module> module_;
  ConversionFunction from_base_;
  ConversionFunction to_base_;
  SerializeFunction serialize_function_;
  DeserializeFunction deserialize_function_;
  std::vector<std::weak_ptr<Function>> constructor_functions_;

  static std::unordered_map<std::type_index, std::weak_ptr<Type>> type_associations;
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
  Value(const Value& other) = default;
  Value(Value&& other) : type_(std::move(other.type_)), data_(std::move(other.data_)), is_view_(other.is_view_) {}

  // Named constructor
  static Value None() { return Value({}, {}, false); }

  template <ReferenceType T>
  static Value Create(T& value);

  template <ReferenceType T>
  static Value Create(T* value);

  template <ValueType T>
  static Value Create(T&& value);

  template <ValueType T>
  static Value Create(T* value);

  static Value Create(const Value& value);

  template <ReferenceType T>
  static Value CreateView(T& value);

  template <ReferenceType T>
  static Value CreateView(T* value);

  template <ValueType T>
  static Value CreateView(T& value);

  template <ValueType T>
  static Value CreateView(T* value);

  static Value CreateView(Value& value);

  template <typename T>
  static Value CreateView(T&& value, std::shared_ptr<Type> actual_type);

  template <typename T> 
  static Value CreateViewIfPossible(T&& value);

  Value& operator=(const Value& other) = default;
  Value& operator=(Value&& other) = default;

  template <ReferenceType T> T& Get();
  template <PointerToReferenceType T> T Get();
  template <ValueType T> std::remove_cvref_t<T>& Get();
  template <PointerToValueType T> T Get();

  template <typename T> requires std::is_pointer_v<T> auto Get() const { return const_cast<Value*>(this)->Get<T>(); }
  template <typename T> requires (!std::is_pointer_v<T>) auto& Get() const { return const_cast<Value*>(this)->Get<T>(); }

  template <typename T>
  void SetProperty(std::string_view property_name, T&& property_value);

  template <typename T = Value>
  T GetProperty(std::string_view property_name);

  json Serialize() const;

  std::weak_ptr<Type> type() const { return type_; }
  bool is_view() const { return is_view_; }
  bool is_none() const { return type_.expired(); }

 private:
  Value(std::weak_ptr<Type> type, std::any data, bool is_view) : type_(type), data_(std::move(data)), is_view_(is_view) {}

  std::weak_ptr<Type> type_;
  std::any data_;
  bool is_view_;

  Value CastToDerived(std::shared_ptr<Type> target_type);
  Value CastToBase(std::shared_ptr<Type> target_type);
};

namespace instructions {
struct NativeFunctionCall {
  void (*function_pointer)(ExecutionContext*);
};
struct PushConstant {
  Value value;
};
struct PushStackValue {
  std::size_t position;
  std::size_t stack_frame_offset;
};
struct Assign {
  std::size_t position;
  std::size_t stack_frame_offset;
};
struct Pop {
  std::size_t count;
};
struct Jump {
  std::ptrdiff_t instruction_offset;
};
struct JumpIfTrue {
  std::ptrdiff_t instruction_offset;
};
struct JumpIfFalse {
  std::ptrdiff_t instruction_offset;
};
struct PushStackFrame {
};
struct PopStackFrame {
};
struct Return {
};
}  // namespace instructions

using Instruction = std::variant<
  instructions::NativeFunctionCall,
  instructions::PushConstant,
  instructions::PushStackValue,
  instructions::Assign,
  instructions::Pop,
  instructions::Jump,
  instructions::JumpIfTrue,
  instructions::JumpIfFalse,
  instructions::PushStackFrame,
  instructions::PopStackFrame,
  instructions::Return
>;

namespace detail {
template <typename T> struct TypeWrapper { using type = T; };
template <typename T> struct TypeWrapper<T&> { using type = std::reference_wrapper<T>; };
template <typename... T> struct TypeWrapper<std::tuple<T...>> { using type = std::tuple<typename TypeWrapper<T>::type...>; };
}
template <typename T> using WrappedType = typename detail::TypeWrapper<T>::type;

class ExecutionContext {
 public:
  ExecutionContext(std::size_t reserved_stack_size = 100);

  // Push an arbitrary value onto the stack. In case T is a tuple, its members are packed onto the stack individually.
  void PushValue2(Value value);
  template <typename T> void PushValue(T&& value);
  template <typename T> void PushValueView(T&& value);

  void PopValue();
  void PopValues(std::size_t count);

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

  std::optional<std::ptrdiff_t> operator()(const instructions::NativeFunctionCall& native_function_call);
  std::optional<std::ptrdiff_t> operator()(const instructions::PushConstant& push_constant);
  std::optional<std::ptrdiff_t> operator()(const instructions::PushStackValue& push_stack_value);
  std::optional<std::ptrdiff_t> operator()(const instructions::Assign& assign);
  std::optional<std::ptrdiff_t> operator()(const instructions::Pop& pop);
  std::optional<std::ptrdiff_t> operator()(const instructions::Jump& jump);
  std::optional<std::ptrdiff_t> operator()(const instructions::JumpIfTrue& jump);
  std::optional<std::ptrdiff_t> operator()(const instructions::JumpIfFalse& jump);
  std::optional<std::ptrdiff_t> operator()(const instructions::PushStackFrame& push_scope);
  std::optional<std::ptrdiff_t> operator()(const instructions::PopStackFrame& pop_scope);
  std::optional<std::ptrdiff_t> operator()(const instructions::Return& jump);

  static ExecutionContext* global_context() { return &global; }

 private:
  struct StackFrame {
    std::size_t base_position;
  };

  std::vector<Value> stack_;
  // Every exuction frame always has the base stack frame that cannot be popped
  // which simplifies the code
  std::vector<StackFrame> stack_frames_;

  static ExecutionContext global;
};

using FunctionPointer = void (*)(ExecutionContext*);

template <typename... T> struct FunctionResult { using type = std::tuple<T...>; };
template <typename T> struct FunctionResult <T> { using type = T; };
template <> struct FunctionResult <> { using type = void; };
template <typename... T> using FunctionResultType = typename FunctionResult<T...>::type;

class Function : public std::enable_shared_from_this<Function> {
  friend class Module;

 public:
  struct ValueDeclaration {
    std::string name;
    std::weak_ptr<Type> type;
  };

  std::string_view name() const { return name_; }
  std::string_view text() const { return text_; }
  FunctionPointer pointer() const { return function_pointer_; }

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
  Function(std::string_view name, FunctionPointer function_pointer, std::vector<ValueDeclaration> inputs,
           std::vector<ValueDeclaration> outputs);

  std::string name_;
  std::string text_;
  FunctionPointer function_pointer_;
  std::vector<ValueDeclaration> inputs_;
  std::vector<ValueDeclaration> outputs_;
};

class Module : public std::enable_shared_from_this<Module> {
 public:
  static std::shared_ptr<Module> Register(std::string_view name);
  static void Deregister(std::string_view name);
  static std::shared_ptr<Module> Get(std::string_view name);
  static std::span<std::shared_ptr<Module>> registered_modules() { return modules; }

  std::string_view name() const { return name_; }

  // Types
  std::shared_ptr<Type> RegisterType(std::string_view name);
  std::shared_ptr<Type> RegisterType(std::string_view name, std::shared_ptr<Type> parent_type,
                                     Type::ConversionFunction from_base, Type::ConversionFunction to_base);

  template <typename T, typename ParentType = void>
  std::shared_ptr<Type> RegisterType(std::string_view name, bool create_cpp_association = true);

  std::shared_ptr<Type> GetType(std::string_view name);
  std::span<std::shared_ptr<Type>> types() { return types_; }
  std::span<const std::shared_ptr<Type>> types() const { return types_; }

  // Functions
  std::shared_ptr<Function> RegisterFunction(std::string_view name, FunctionPointer function_pointer,
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
  std::vector<std::shared_ptr<Type>> types_;
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
