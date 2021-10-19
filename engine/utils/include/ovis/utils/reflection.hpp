#pragma once

#include <any>
#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <span>
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

class Type : public SafelyReferenceable {
  friend class Value;

 public:
  struct Property {
    using GetFunction = std::add_pointer_t<Value(const Value& object)>;
    using SetFunction = std::add_pointer_t<void(Value* object, const Value& property_value)>;

    safe_ptr<Type> type;
    std::string name;
    GetFunction getter;
    SetFunction setter;
  };

  static safe_ptr<Type> Register(std::string_view name);
  template <typename T, typename ParentType = void> static safe_ptr<Type> Register(std::string_view name);
  static bool Deregister(std::string_view);
  static void DeregisterAll() { types.clear(); } // Useful for testing
  static safe_ptr<Type> Get(std::string_view name);
  template <typename T> static safe_ptr<Type> Get();

  std::string_view name() const { return name_; }
  Type* parent() const { return parent_.get(); }
  std::optional<std::type_index> associated_type() { return associated_type_; }
  std::span<const Property> properties() const { return properties_; }

  void RegisterProperty(std::string_view name, Type* type, Property::GetFunction getter,
                        Property::SetFunction setter = nullptr);
  template <auto PROPERTY> void RegisterProperty(std::string_view);

 private:
  Type(std::string_view name, Type* parent = nullptr);
  Type(std::string_view name, const std::type_info& associated_type, Type* parent = nullptr);

  std::vector<Property> properties_;

  std::string name_; 
  std::optional<std::type_index> associated_type_;
  safe_ptr<Type> parent_;

  static std::vector<Type> types;
};

class Value {
 public:
   Value() : type_(nullptr) {}
   template <typename T> Value(T&& value);

   template <typename T> std::remove_cvref_t<T>& Get();
   template <typename T> const std::remove_cvref_t<T>& Get() const;

   void SetProperty(std::string_view property_name, const Value& property_value);
   template <typename T> void SetProperty(std::string_view property_name, T&& property_value);

   Value GetProperty(std::string_view property_name);
   template <typename T> T GetProperty(std::string_view property_name);

   Type* type() const { return type_.get(); }

 private:
   safe_ptr<Type> type_;
   std::any data_;
};

class Function : public SafelyReferenceable {
 public:
  struct ValueDeclaration {
    std::string name;
    safe_ptr<Type> type;
  };
  using Pointer = void(*)(std::span<const Value> inputs, std::span<Value> outputs);

  static safe_ptr<Function> Register(std::string_view name, Pointer function_pointer,
                                     std::vector<ValueDeclaration> inputs, std::vector<ValueDeclaration> outputs);
  template <auto FUNCTION>
  static safe_ptr<Function> Register(std::string_view name, std::vector<std::string_view> input_names,
                                     std::vector<std::string_view> output_names);
  static void DeregisterAll() { functions.clear(); } // Useful for testing
  static safe_ptr<Function> Get(std::string_view name);

  static std::vector<Value> Call(std::string_view name, std::span<const Value> inputs = {});

  std::vector<Value> Call(std::span<const Value> inputs = {});

 private:
  Function(std::string_view name, Pointer function_pointer, std::vector<ValueDeclaration> inputs,
           std::vector<ValueDeclaration> outputs);

  std::string name_;
  Pointer function_pointer_;
  std::vector<ValueDeclaration> inputs_;
  std::vector<ValueDeclaration> outputs_;

  static std::vector<Function> functions;
};

}

// Implementation
namespace ovis {

inline Type::Type(std::string_view name, Type* parent) : name_(name), parent_(parent) {}
inline Type::Type(std::string_view name, const std::type_info& associated_type, Type* parent)
    : name_(name), associated_type_(associated_type), parent_(parent) {}

inline safe_ptr<Type> Type::Register(std::string_view name) {
  if (Get(name) != nullptr) {
    return nullptr;
  }

  types.push_back(Type(name));
  return safe_ptr(&types.back());
}

template <typename T, typename ParentType>
inline safe_ptr<Type> Type::Register(std::string_view name) {
  static_assert(std::is_same_v<ParentType, void>);
  if (Get<T>() != nullptr) {
    return nullptr;
  }

  auto type = Register(name);
  if (type == nullptr) {
    return nullptr;
  }

  type->associated_type_ = std::type_index(typeid(T));
  return type;
}

inline bool Type::Deregister(std::string_view name) {
  auto new_end = std::remove_if(types.begin(), types.end(), [name](const Type& type) { return type.name() == name; });
  
  if (new_end == types.end()) {
    return false;
  } else {
    assert(types.end() - new_end == 1);
    types.erase(new_end, types.end());
    return true;
  }
}

inline safe_ptr<Type> Type::Get(std::string_view name) {
  for (auto& type : types) {
    if (type.name() == name) {
      return safe_ptr(&type);
    }
  }
  return nullptr;
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

template <typename T>
inline safe_ptr<Type> Type::Get() {
  for (auto& type : types) {
    if (type.associated_type() == std::type_index(typeid(std::remove_cvref_t<T>))) {
      return safe_ptr(&type);
    }
  }
  return nullptr;
}

namespace detail {

template <auto PROPERTY> class PropertyCallbacks {};

template <typename T, typename PropertyType, PropertyType T::* PROPERTY>
struct PropertyCallbacks<PROPERTY> {
  static Value PropertyGetter(const ovis::Value& object) {
    return object.Get<T>().*PROPERTY;
  }

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

inline safe_ptr<Function> Function::Register(std::string_view name, Pointer function_pointer,
                                             std::vector<ValueDeclaration> inputs,
                                             std::vector<ValueDeclaration> outputs) {
  if (Get(name) != nullptr) {
    return nullptr;
  }
  functions.push_back(Function(name, function_pointer, std::forward<std::vector<ValueDeclaration>>(inputs),
                               std::forward<std::vector<ValueDeclaration>>(outputs)));
  return safe_ptr(&functions.back());
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

template <typename... T,  std::size_t... I>
inline void SetFunctionTupleOutputsImpl(std::span<Value> outputs, std::tuple<T...>&& values, std::index_sequence<I...>) {
  assert(outputs.size() == sizeof...(T));
  ((outputs[I] = std::get<I>(values)), ...);
}
template <typename... T,  typename Indices = std::make_index_sequence<sizeof...(T)>>
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

template <typename FunctionType> struct FunctionWrapper;

template <typename ReturnType, typename... ArgumentTypes>
struct FunctionWrapper<ReturnType(*)(ArgumentTypes...)> {
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
inline safe_ptr<Function> Function::Register(std::string_view name, std::vector<std::string_view> input_names,
                                             std::vector<std::string_view> output_names) {
  std::vector<ValueDeclaration> inputs(input_names.size());
  std::vector<ValueDeclaration> outputs(output_names.size());
  return Register(name, &detail::FunctionWrapper<decltype(FUNCTION)>::template Call<FUNCTION>, std::move(inputs),
                  std::move(outputs));
}

inline safe_ptr<Function> Function::Get(std::string_view name) {
  for (auto& function : functions) {
    if (function.name_ == name) {
      return safe_ptr(&function);
    }
  }
  return nullptr;
}

inline std::vector<Value> Function::Call(std::string_view name, std::span<const Value> inputs) {
  auto function = Get(name);
  assert(function != nullptr);
  return function->Call(inputs);
}

inline std::vector<Value> Function::Call(std::span<const Value> inputs) {
  assert(inputs.size() == inputs_.size());

  std::vector<Value> outputs(outputs_.size());
  function_pointer_(inputs, outputs);
  return outputs;
}

inline Function::Function(std::string_view name, Pointer function_pointer, std::vector<ValueDeclaration> inputs,
                          std::vector<ValueDeclaration> outputs)
    : name_(name), function_pointer_(function_pointer), inputs_(inputs), outputs_(outputs) {}

}  // namespace ovis

