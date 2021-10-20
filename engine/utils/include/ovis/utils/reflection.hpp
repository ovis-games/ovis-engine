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
#include <ovis/utils/safe_pointer.hpp>

namespace ovis {

// Forward declarations
class Type;
class Value;
class Function;
class Module;

class Type : public SafelyReferenceable {
  friend class Value;
  friend class Module;

 public:
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
  std::span<const Property> properties() const { return properties_; }

  void RegisterProperty(std::string_view name, Type* type, Property::GetFunction getter,
                        Property::SetFunction setter = nullptr);
  template <auto PROPERTY>
  void RegisterProperty(std::string_view);

  template <typename T>
  static safe_ptr<Type> Get();

 private:
  Type(std::string_view name, Type* parent = nullptr);

  std::vector<Property> properties_;

  std::string name_;
  safe_ptr<Type> parent_;

  static std::unordered_map<std::type_index, safe_ptr<Type>> type_associations;
};

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

  Type* type() const { return type_.get(); }

 private:
  safe_ptr<Type> type_;
  std::any data_;
};

class Function : public SafelyReferenceable {
  friend class Module;

 public:
  struct ValueDeclaration {
    std::string name;
    safe_ptr<Type> type;
  };
  using Pointer = void (*)(std::span<const Value> inputs, std::span<Value> outputs);

  std::span<const ValueDeclaration> inputs() const { return inputs_; }
  std::span<const ValueDeclaration> outputs() const { return outputs_; }

  std::vector<Value> Call(std::span<const Value> inputs = {});

 private:
  Function(std::string_view name, Pointer function_pointer, std::vector<ValueDeclaration> inputs,
           std::vector<ValueDeclaration> outputs);

  std::string name_;
  Pointer function_pointer_;
  std::vector<ValueDeclaration> inputs_;
  std::vector<ValueDeclaration> outputs_;
};

class Module : public SafelyReferenceable {
 public:
  static safe_ptr<Module> Register(std::string_view name);
  static safe_ptr<Module> Get(std::string_view name);

  // Types
  safe_ptr<Type> RegisterType(std::string_view name);

  template <typename T, typename ParentType = void>
  safe_ptr<Type> RegisterType(std::string_view name, bool create_cpp_association = true);

  safe_ptr<Type> GetType(std::string_view name);

  // Functions
  safe_ptr<Function> RegisterFunction(std::string_view name, Function::Pointer function_pointer,
                                      std::vector<Function::ValueDeclaration> inputs,
                                      std::vector<Function::ValueDeclaration> outputs);

  template <auto FUNCTION>
  safe_ptr<Function> RegisterFunction(std::string_view name, std::vector<std::string_view> input_names,
                                      std::vector<std::string_view> output_names);

  safe_ptr<Function> GetFunction(std::string_view name);

  std::vector<Value> CallFunction(std::string_view name, std::span<const Value> inputs = {});

 private:
  std::string name_;
  std::vector<Type> types_;
  std::vector<Function> functions_;
};
}  // namespace ovis

// Implementation
namespace ovis {

// Type
inline Type::Type(std::string_view name, Type* parent) : name_(name), parent_(parent) {}

template <typename T>
inline safe_ptr<Type> Type::Get() {
  if (auto it = type_associations.find(typeid(T)); it != type_associations.end()) {
    return it->second;
  } else {
    return nullptr;
  }
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
  static Value PropertyGetter(const ovis::Value& object) { return object.Get<T>().*PROPERTY; }

  static void PropertySetter(ovis::Value* object, const ovis::Value& property_value) {
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
}

template <typename T>
inline T Value::GetProperty(std::string_view property_name) {
  return GetProperty(property_name).Get<T>();
}

inline std::vector<Value> Function::Call(std::span<const Value> inputs) {
  assert(inputs.size() == inputs_.size());

  std::vector<Value> outputs(outputs_.size());
  function_pointer_(inputs, outputs);
  return outputs;
}

// Value
inline Function::Function(std::string_view name, Pointer function_pointer, std::vector<ValueDeclaration> inputs,
                          std::vector<ValueDeclaration> outputs)
    : name_(name), function_pointer_(function_pointer), inputs_(inputs), outputs_(outputs) {}

inline safe_ptr<Type> Module::RegisterType(std::string_view name) {
  if (GetType(name) != nullptr) {
    return nullptr;
  }

  types_.push_back(Type(name));
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

inline safe_ptr<Function> Module::RegisterFunction(std::string_view name, Function::Pointer function_pointer,
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

template <std::size_t N, typename... Types>
using nth_parameter_t = std::tuple_element_t<N, std::tuple<Types...>>;
static_assert(std::is_same_v<void, nth_parameter_t<0, void, bool, int>>);
static_assert(std::is_same_v<bool, nth_parameter_t<1, void, bool, int>>);
static_assert(std::is_same_v<int, nth_parameter_t<2, void, bool, int>>);

// ConstructArgumentTuple()
template <typename... ArgumentTypes, std::size_t... I>
std::tuple<ArgumentTypes...> ConstructArgumentTupleImpl(std::span<const Value> inputs, std::index_sequence<I...>) {
  return std::make_tuple(inputs[I].Get<nth_parameter_t<I, ArgumentTypes...>>()...);
}
template <typename... ArgumentTypes, typename Indices = std::make_index_sequence<sizeof...(ArgumentTypes)>>
std::tuple<ArgumentTypes...> ConstructArgumentTuple(std::span<const Value> inputs) {
  return ConstructArgumentTupleImpl<ArgumentTypes...>(inputs, Indices{});
}

template <typename... T, std::size_t... I>
inline void SetFunctionTupleOutputsImpl(std::span<Value> outputs, std::tuple<T...>&& values,
                                        std::index_sequence<I...>) {
  assert(outputs.size() == sizeof...(T));
  ((outputs[I] = std::get<I>(values)), ...);
}
template <typename... T, typename Indices = std::make_index_sequence<sizeof...(T)>>
inline void SetFunctionTupleOutputs(std::span<Value> outputs, std::tuple<T...>&& values) {
  SetFunctionTupleOutputsImpl(outputs, std::forward<std::tuple<T...>>(values), Indices{});
}

template <typename T>
inline void SetFunctionOutputs(std::span<Value> outputs, T&& value) {
  assert(outputs.size() == 1);
  outputs[0] = std::move(value);
}

template <typename... T>
inline void SetFunctionOutputs(std::span<Value> outputs, std::tuple<T...>&& values) {
  SetFunctionTupleOutputs(outputs, std::forward<decltype(values)>(values));
}

template <typename FunctionType>
struct FunctionWrapper;

template <typename ReturnType, typename... ArgumentTypes>
struct FunctionWrapper<ReturnType (*)(ArgumentTypes...)> {
  using FunctionPointerType = ReturnType (*)(ArgumentTypes...);
  using ArgumentTuple = std::tuple<ArgumentTypes...>;

  template <FunctionPointerType FUNCTION>
  static void Call(std::span<const Value> inputs, std::span<Value> outputs) {
    assert(inputs.size() == sizeof...(ArgumentTypes));
    if constexpr (std::is_same_v<ReturnType, void>) {
      assert(outputs.size() == 0);
      std::apply(FUNCTION, ConstructArgumentTuple<ArgumentTypes...>(inputs));
    } else {
      SetFunctionOutputs(outputs, std::apply(FUNCTION, ConstructArgumentTuple<ArgumentTypes...>(inputs)));
    }
  }
};

}  // namespace detail

template <auto FUNCTION>
inline safe_ptr<Function> Module::RegisterFunction(std::string_view name, std::vector<std::string_view> input_names,
                                                   std::vector<std::string_view> output_names) {
  std::vector<Function::ValueDeclaration> inputs(input_names.size());
  std::vector<Function::ValueDeclaration> outputs(output_names.size());
  return RegisterFunction(name, &detail::FunctionWrapper<decltype(FUNCTION)>::template Call<FUNCTION>,
                          std::move(inputs), std::move(outputs));
}

inline safe_ptr<Function> Module::GetFunction(std::string_view name) {
  for (auto& function : functions_) {
    if (function.name_ == name) {
      return safe_ptr(&function);
    }
  }
  return nullptr;
}

inline std::vector<Value> Module::CallFunction(std::string_view name, std::span<const Value> inputs) {
  auto function = GetFunction(name);
  assert(function != nullptr);
  return function->Call(inputs);
}

}  // namespace ovis

