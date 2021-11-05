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

class Type : public SafelyReferenceable {
  friend class Value;
  friend class Module;

 public:
  using SerializeFunction = json(*)(const Value& value);
  using DeserializeFunction = Value(*)(const json& value);

  struct Property {
    using GetFunction = std::add_pointer_t<Value(const Value& object)>;
    using SetFunction = std::add_pointer_t<void(Value* object, const Value& property_value)>;

    safe_ptr<Type> type;
    std::string name;
    GetFunction getter;
    SetFunction setter;
  };

  std::string_view name() const { return name_; }
  Type* parent() const { return parent_.get(); }
  Module* module() const { return module_.get(); }

  void SetSerializeFunction(SerializeFunction function) { serialize_function_ = function; }
  SerializeFunction serialize_function() const { return serialize_function_; }

  void SetDeserializeFunction(DeserializeFunction function) { deserialize_function_ = function; }
  DeserializeFunction deserialize_function() const { return deserialize_function_; }

  void RegisterProperty(std::string_view name, Type* type, Property::GetFunction getter,
                        Property::SetFunction setter = nullptr);
  template <auto PROPERTY>
  void RegisterProperty(std::string_view);
  std::span<const Property> properties() const { return properties_; }

  Value CreateValue(const json& data) const;

  template <typename T>
  static safe_ptr<Type> Get();

  static safe_ptr<Type> Deserialize(const json& data);

 private:
  Type(Module* module, std::string_view name, Type* parent = nullptr);

  std::vector<Property> properties_;

  std::string name_;
  safe_ptr<Type> parent_;
  safe_ptr<Module> module_;
  SerializeFunction serialize_function_;
  DeserializeFunction deserialize_function_;

  static std::unordered_map<std::type_index, safe_ptr<Type>> type_associations;
};
std::ostream& operator<<(std::ostream& os, const safe_ptr<Type>& pointer);

class Value {
 public:
  Value() : type_(nullptr) {}
  template <typename T>
  Value(T&& value);

  template <typename T>
  std::remove_cvref_t<T>& Get();
  template <typename T>
  const std::remove_cvref_t<T>& Get() const;

  void SetProperty(std::string_view property_name, const Value& property_value);
  template <typename T>
  void SetProperty(std::string_view property_name, T&& property_value);

  Value GetProperty(std::string_view property_name);
  template <typename T>
  T GetProperty(std::string_view property_name);

  json Serialize() const;

  Type* type() const { return type_.get(); }

  static Value None() { return Value(); }

 private:
  safe_ptr<Type> type_;
  std::any data_;
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

class ExecutionContext {
 public:
  ExecutionContext(std::size_t reserved_stack_size = 100);

  // Push an arbitrary value onto the stack. In case T is a tuple, its members are packed onto the stack individually.
  template <typename T> void PushValue(T&& value);

  void PopValue();
  void PopValues(std::size_t count);

  void PushStackFrame();
  void PopStackFrame();

  Value& GetValue(std::size_t position, std::size_t stack_frame_offset = 0);

  // Returns an arbitrary value from the stack. In case T is a tuple it will be filled with the values at positions
  // [position, position + tuple_size_v<T>).
  template <typename T> T GetValue(std::size_t position, std::size_t stack_frame_offset = 0);

  Value& GetTopValue(std::size_t offset_from_top = 0);
  // Returns an arbitrary value from the top of the stack. In case T is a tuple it will be filled with the values at
  // offsets: (offset_from_top + tuple_size_v<T>, offset_from_top).
  template <typename T> T GetTopValue(std::size_t offset_from_top = 0);

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

class Function : public SafelyReferenceable {
  friend class Module;

 public:
  struct ValueDeclaration {
    std::string name;
    safe_ptr<Type> type;
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

  template <typename... OutputTypes, typename... InputsTypes>
  FunctionResultType<OutputTypes...> Call(InputsTypes&&... inputs);
  template <typename... OutputTypes, typename... InputsTypes>
  FunctionResultType<OutputTypes...> Call(ExecutionContext* context, InputsTypes&&... inputs);

  static safe_ptr<Function> Deserialize(const json& data);

 private:
  Function(std::string_view name, FunctionPointer function_pointer, std::vector<ValueDeclaration> inputs,
           std::vector<ValueDeclaration> outputs);

  std::string name_;
  std::string text_;
  FunctionPointer function_pointer_;
  std::vector<ValueDeclaration> inputs_;
  std::vector<ValueDeclaration> outputs_;
};

class Module : public SafelyReferenceable {
 public:
  static safe_ptr<Module> Register(std::string_view name);
  static void Deregister(std::string_view name);
  static safe_ptr<Module> Get(std::string_view name);
  static std::span<Module> registered_modules() { return modules; }

  std::string_view name() const { return name_; }

  // Types
  safe_ptr<Type> RegisterType(std::string_view name);

  template <typename T, typename ParentType = void>
  safe_ptr<Type> RegisterType(std::string_view name, bool create_cpp_association = true);

  safe_ptr<Type> GetType(std::string_view name);
  std::span<Type> types() { return types_; }
  std::span<const Type> types() const { return types_; }

  // Functions
  safe_ptr<Function> RegisterFunction(std::string_view name, FunctionPointer function_pointer,
                                      std::vector<Function::ValueDeclaration> inputs,
                                      std::vector<Function::ValueDeclaration> outputs);

  template <auto FUNCTION>
  safe_ptr<Function> RegisterFunction(std::string_view name, std::vector<std::string_view> input_names,
                                      std::vector<std::string_view> output_names);

  safe_ptr<Function> GetFunction(std::string_view name);
  std::span<Function> functions() { return functions_; }
  std::span<const Function> functions() const { return functions_; }

 private:
  Module(std::string_view name) : name_(name) {}

  std::string name_;
  std::vector<Type> types_;
  std::vector<Function> functions_;

  static std::vector<Module> modules;
};
}  // namespace vm
}  // namespace ovis

// Implementation
namespace ovis {
namespace vm {

// Type
inline Type::Type(Module* module, std::string_view name, Type* parent)
    : module_(module), name_(name), parent_(parent), serialize_function_(nullptr), deserialize_function_(nullptr) {}

template <typename T>
inline safe_ptr<Type> Type::Get() {
  if (auto it = type_associations.find(typeid(T)); it != type_associations.end()) {
    return it->second;
  } else {
    return nullptr;
  }
}

inline safe_ptr<Type> Type::Deserialize(const json& data) {
  if (!data.contains("module")) {
    return nullptr;
  }
  const auto& module_json = data.at("module");
  if (!module_json.is_string()) {
    return nullptr;
  }
  const safe_ptr<vm::Module> module = Module::Get(module_json.get_ref<const std::string&>());
  if (module == nullptr) {
    return nullptr;
  }
  if (!data.contains("name")) {
    return nullptr;
  }
  const auto& name_json = data.at("name");
  if (!name_json.is_string()) {
    return nullptr;
  }
  return module->GetType(name_json.get_ref<const std::string&>());
}

inline void Type::RegisterProperty(std::string_view name, Type* type, Property::GetFunction getter,
                                   Property::SetFunction setter) {
  properties_.push_back({
      .type = safe_ptr(type),
      .name = std::string(name),
      .getter = getter,
      .setter = setter,
  });
}

namespace detail {

template <auto PROPERTY>
class PropertyCallbacks {};

template <typename T, typename PropertyType, PropertyType T::*PROPERTY>
struct PropertyCallbacks<PROPERTY> {
  static Value PropertyGetter(const ovis::vm::Value& object) { return object.Get<T>().*PROPERTY; }

  static void PropertySetter(ovis::vm::Value* object, const ovis::vm::Value& property_value) {
    assert(property_value.type() == Type::template Get<PropertyType>());
    object->Get<T>().*PROPERTY = property_value.Get<PropertyType>();
  }

  static void Register(Type* type, std::string_view name) {
    type->RegisterProperty(name, Type::Get<PropertyType>().get(), &PropertyGetter, &PropertySetter);
  }
};

}  // namespace detail

template <auto PROPERTY>
inline void Type::RegisterProperty(std::string_view name) {
  detail::PropertyCallbacks<PROPERTY>::Register(this, name);
}

inline Value Type::CreateValue(const json& data) const {
  if (deserialize_function_) {
    return deserialize_function_(data);
  } else {
    return Value::None();
  }
}

// Value
template <typename T>
Value::Value(T&& value) : type_(Type::Get<T>()), data_(std::move(value)) {}

template <typename T>
std::remove_cvref_t<T>& Value::Get() {
  assert(type_ == Type::Get<T>());
  return std::any_cast<std::remove_cvref_t<T>&>(data_);
}

template <typename T>
const std::remove_cvref_t<T>& Value::Get() const {
  return std::any_cast<const std::remove_cvref_t<T>&>(data_);
}

inline void Value::SetProperty(std::string_view property_name, const Value& property_value) {
  for (const auto& property : type_->properties_) {
    if (property.name == property_name) {
      assert(property.type == property_value.type());
      property.setter(this, property_value);
      return;
    }
  }
  assert(false);
}

template <typename T>
inline void Value::SetProperty(std::string_view property_name, T&& property_value) {
  const Value value = Value{std::forward<T>(property_value)};
  SetProperty(property_name, value);
}

inline Value Value::GetProperty(std::string_view property_name) {
  for (const auto& property : type_->properties_) {
    if (property.name == property_name) {
      return property.getter(*this);
    }
  }
  assert(false);
  return {};
}

template <typename T>
inline T Value::GetProperty(std::string_view property_name) {
  return GetProperty(property_name).Get<T>();
}

inline json Value::Serialize() const {
  if (type() && type()->serialize_function()) {
    return type()->serialize_function()(*this);
  } else {
    return json();
  }
}

// Function

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

inline safe_ptr<Function> Function::Deserialize(const json& data) {
  if (!data.contains("module")) {
    return nullptr;
  }
  const auto& module_json = data.at("module");
  if (!module_json.is_string()) {
    return nullptr;
  }
  const safe_ptr<vm::Module> module = Module::Get(module_json.get_ref<const std::string&>());
  if (module == nullptr) {
    return nullptr;
  }
  if (!data.contains("name")) {
    return nullptr;
  }
  const auto& name_json = data.at("name");
  if (!name_json.is_string()) {
    return nullptr;
  }
  return module->GetFunction(name_json.get_ref<const std::string&>());
}

// ExecutionContext
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

  static T Get(ExecutionContext* context, std::size_t position, std::size_t stack_frame_offset) {
    return context->GetValue(position, stack_frame_offset).Get<T>();
  }

  static T GetTop(ExecutionContext* context, std::size_t offset_from_top) {
    return context->GetTopValue(offset_from_top).Get<T>();
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
  static std::tuple<T...> GetTupleValueImpl(ExecutionContext* context, std::size_t base_position,
                                            std::size_t stack_frame_offset, std::index_sequence<I...>) {
    return std::make_tuple(context->GetValue<nth_parameter_t<I, T...>>(base_position + I, stack_frame_offset)...);
  }
  template <typename Indices = std::make_index_sequence<sizeof...(T)>>
  static std::tuple<T...> GetTupleValue(ExecutionContext* context, std::size_t base_position, std::size_t stack_frame_offset) {
    return GetTupleValueImpl(context, base_position, stack_frame_offset, Indices{});
  }

  static std::tuple<T...> Get(ExecutionContext* context, std::size_t position, std::size_t stack_frame_offset) {
    return GetTupleValue(context, position, stack_frame_offset);
  }

  template <std::size_t... I>
  static std::tuple<T...> GetTopTupleImpl(ExecutionContext* context, std::size_t offset_from_top, std::index_sequence<I...>) {
    return std::make_tuple(context->GetTopValue(offset_from_top + sizeof...(I) - I - 1).Get<nth_parameter_t<I, T...>>()...);
  }
  template <typename Indices = std::make_index_sequence<sizeof...(T)>>
  static std::tuple<T...> GetTopTuple(ExecutionContext* context, std::size_t offset_from_top) {
    return GetTopTupleImpl(context, offset_from_top, Indices{});
  }
  static std::tuple<T...> GetTop(ExecutionContext* context, std::size_t offset_from_top) {
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
inline T ExecutionContext::GetValue(std::size_t position, std::size_t stack_frame_offset) {
  return detail::ValueHelper<T>::Get(this, position, stack_frame_offset);
}

inline Value& ExecutionContext::GetTopValue(std::size_t offset_from_top) {
  assert(stack_frames_.size() > 0);
  assert(offset_from_top < stack_.size());
  assert(offset_from_top < stack_.size() - stack_frames_.back().base_position);
  return *(stack_.rbegin() + offset_from_top);
}

template <typename T>
inline T ExecutionContext::GetTopValue(std::size_t offset_from_top) {
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

// Function
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

inline Function::Function(std::string_view name, FunctionPointer function_pointer, std::vector<ValueDeclaration> inputs,
                          std::vector<ValueDeclaration> outputs)
    : name_(name), function_pointer_(function_pointer), inputs_(inputs), outputs_(outputs) {
  text_ = name_;
  for (const auto input: inputs_) {
    text_ += fmt::format(" ({})", input.name);
  }
}

// Module
inline safe_ptr<Module> Module::Register(std::string_view name) {
  if (Get(name) != nullptr) {
    return nullptr;
  }

  modules.push_back(Module(name));
  return safe_ptr(&modules.back());
}

inline void Module::Deregister(std::string_view name) {
  std::erase_if(modules, [name](const Module& module) {
    return module.name_ == name;
  });
}

inline safe_ptr<Module> Module::Get(std::string_view name) {
  for (auto& module : modules) {
    if (module.name_ == name) {
      return safe_ptr(&module);
    }
  }
  return nullptr;
}

inline safe_ptr<Type> Module::RegisterType(std::string_view name) {
  if (GetType(name) != nullptr) {
    return nullptr;
  }

  types_.push_back(Type(this, name));
  return safe_ptr(&types_.back());
}

template <typename T, typename ParentType>
inline safe_ptr<Type> Module::RegisterType(std::string_view name, bool create_cpp_association) {
  static_assert(std::is_same_v<ParentType, void>);
  if (Type::Get<T>() != nullptr) {
    return nullptr;
  }

  if (create_cpp_association && Type::type_associations[typeid(T)] != nullptr) {
    return nullptr;
  }

  auto type = RegisterType(name);
  if (type == nullptr) {
    return nullptr;
  }

  if (create_cpp_association) {
    Type::type_associations[typeid(T)] = type;
  }

  return type;
}

inline safe_ptr<Type> Module::GetType(std::string_view name) {
  for (auto& type : types_) {
    if (type.name() == name) {
      return safe_ptr(&type);
    }
  }
  return nullptr;
}

inline safe_ptr<Function> Module::RegisterFunction(std::string_view name, FunctionPointer function_pointer,
                                                   std::vector<Function::ValueDeclaration> inputs,
                                                   std::vector<Function::ValueDeclaration> outputs) {
  if (GetFunction(name) != nullptr) {
    return nullptr;
  }
  functions_.push_back(Function(name, function_pointer, std::forward<std::vector<Function::ValueDeclaration>>(inputs),
                                std::forward<std::vector<Function::ValueDeclaration>>(outputs)));
  return safe_ptr(&functions_.back());
}

namespace detail {


template <typename T>
struct VariableDeclarationsHelper {
  static void Insert(std::vector<Function::ValueDeclaration>* declarations, std::span<std::string_view> variable_names) {
    assert(declarations->size() < variable_names.size());
    declarations->push_back({
        std::string(variable_names[declarations->size()]),
        Type::Get<T>()
    });
  }
};
template <>
struct VariableDeclarationsHelper<void> {
  static void Insert(std::vector<Function::ValueDeclaration>* declarations, std::span<std::string_view> variable_names) {
  }
};
template <typename... T>
struct VariableDeclarationsHelper<std::tuple<T...>> {
  static void Insert(std::vector<Function::ValueDeclaration>* declarations, std::span<std::string_view> variable_names) {
    ((VariableDeclarationsHelper<T>::Insert(declarations, variable_names)), ...);
  }
};

template <typename FunctionType>
struct FunctionWrapper;

template <typename ReturnType, typename... ArgumentTypes>
struct FunctionWrapper<ReturnType (*)(ArgumentTypes...)> {
  using FunctionPointerType = ReturnType (*)(ArgumentTypes...);
  using ArgumentTuple = std::tuple<ArgumentTypes...>;

  static std::vector<Function::ValueDeclaration> GetInputDeclarations(std::span<std::string_view> input_names) {
    std::vector<Function::ValueDeclaration> input_declarations;
    VariableDeclarationsHelper<ArgumentTuple>::Insert(&input_declarations, input_names);
    assert(input_declarations.size() == input_names.size());
    return input_declarations;
  }

  static std::vector<Function::ValueDeclaration> GetOutputDeclarations(std::span<std::string_view> output_names) {
    std::vector<Function::ValueDeclaration> output_declarations;
    VariableDeclarationsHelper<ReturnType>::Insert(&output_declarations, output_names);
    assert(output_declarations.size() == output_names.size());
    return output_declarations;
  }

  template <FunctionPointerType FUNCTION>
  static void Call(ExecutionContext* context) {
    if constexpr (std::is_same_v<ReturnType, void>) {
      std::apply(FUNCTION, context->GetValue<ArgumentTuple>(0));
    } else {
      context->PushValue(std::apply(FUNCTION, context->GetValue<ArgumentTuple>(0)));
    }
  }
};

}  // namespace detail

template <auto FUNCTION>
inline safe_ptr<Function> Module::RegisterFunction(std::string_view name, std::vector<std::string_view> input_names,
                                                   std::vector<std::string_view> output_names) {
  return RegisterFunction(
      name,
      &detail::FunctionWrapper<decltype(FUNCTION)>::template Call<FUNCTION>,
      detail::FunctionWrapper<decltype(FUNCTION)>::GetInputDeclarations(input_names), 
      detail::FunctionWrapper<decltype(FUNCTION)>::GetOutputDeclarations(output_names));
}

inline safe_ptr<Function> Module::GetFunction(std::string_view name) {
  for (auto& function : functions_) {
    if (function.name_ == name) {
      return safe_ptr(&function);
    }
  }
  return nullptr;
}

}  // namespace vm
}  // namespace ovis

