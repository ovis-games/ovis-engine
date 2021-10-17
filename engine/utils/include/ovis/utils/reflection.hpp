#pragma once

#include <any>
#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <vector>

#include <ovis/utils/down_cast.hpp>

namespace ovis {

// Forward declarations
class Type;
template <typename T> class NativeType;
class Value;

class Type {
  friend class Value;

 public:
  struct Property {
    using GetFunction = std::add_pointer_t<Value(const Value& object)>;
    using SetFunction = std::add_pointer_t<void(Value* object, const Value& property_value)>;

    Type* type;
    std::string name;
    GetFunction getter;
    SetFunction setter;
  };

  // static Type* Register(std::string_view name);
  template <typename T, typename ParentType = void> static NativeType<T>* Register(std::string_view name);
  static Type* Get(std::string_view name);
  template <typename T> static NativeType<std::remove_cvref_t<T>>* Get();

  std::string_view name() const { return name_; }
  Type* parent() const { return parent_; }
  std::optional<std::type_index> associated_type() { return associated_type_; }

 protected:
  Type(std::string_view name, Type* parent = nullptr);
  Type(std::string_view name, const std::type_info& associated_type, Type* parent = nullptr);

  std::vector<Property> properties_;

 private:
  std::string name_; 
  std::optional<std::type_index> associated_type_;
  Type* parent_;

  static std::vector<std::shared_ptr<Type>> types;
};

template <typename T>
class NativeType : public Type {
  friend class Type;

 public:
  template <auto PROPERTY> void RegisterProperty(std::string_view);

 private:
  NativeType(std::string_view name);
};

class Value {
 public:
   template <typename T> Value(T&& value);

   template <typename T> T& Get();
   template <typename T> T Get() const;

   void SetProperty(std::string_view property_name, const Value& property_value);
   template <typename T> void SetProperty(std::string_view property_name, T&& property_value);

   Value GetProperty(std::string_view property_name);
   template <typename T> T GetProperty(std::string_view property_name);

   Type* type() const { return type_; }

 private:
   Type* type_;
   std::any data_;
};

}

// Implementation
namespace ovis {

inline Type::Type(std::string_view name, Type* parent) : name_(name), parent_(parent) {}
inline Type::Type(std::string_view name, const std::type_info& associated_type, Type* parent)
    : name_(name), associated_type_(associated_type), parent_(parent) {}

// inline Type* Type::Register(std::string_view name) {
//   // TODO: mutex?
//   static std::vector<std::unique_ptr<Type>> types;
//   std::unique_ptr<Type> type(new Type());
//   types.push_back(std::move(type));
//   return types.back().get();
// }

template <typename T, typename ParentType>
inline NativeType<T>* Type::Register(std::string_view name) {
  static_assert(std::is_same_v<ParentType, void>);
  std::shared_ptr<NativeType<T>> type(new NativeType<T>(name));
  // TODO: mutex?
  types.push_back(type);
  return type.get();
}

inline Type* Type::Get(std::string_view name) {
  for (const auto& type : types) {
    if (type->name() == name) {
      return type.get();
    }
  }
  return nullptr;
}

template <typename T>
inline NativeType<std::remove_cvref_t<T>>* Type::Get() {
  for (const auto& type : types) {
    if (type->associated_type() == std::type_index(typeid(std::remove_cvref_t<T>))) {
      return down_cast<NativeType<std::remove_cvref_t<T>>*>(type.get());
    }
  }
  return nullptr;
}

namespace detail {

template <typename T, auto T::*PROPERTY>
Value PropertyGetter(const ovis::Value& object) {
  return object.Get<T>().*PROPERTY;
}

template <typename T, auto T::*PROPERTY>
void PropertySetter(ovis::Value* object, const ovis::Value& property_value) {
  assert(property_value.type() == Type::template Get<decltype(std::declval<T>().*PROPERTY)>());
  object->Get<T>().*PROPERTY = property_value.Get<decltype(std::declval<T>().*PROPERTY)>();
}

template <typename T, auto T::*PROPERTY>
void AddProperty(std::vector<Type::Property>* properties, std::string_view name) {
  properties->push_back(Type::Property{
      .type = Type::Get<decltype(std::declval<T>().*PROPERTY)>(),
      .name = std::string(name),
      .getter = &detail::PropertyGetter<T, PROPERTY>,
      .setter = &detail::PropertySetter<T, PROPERTY>,
  });
}

}  // namespace detail

template <typename T>
template <auto PROPERTY>
inline void NativeType<T>::RegisterProperty(std::string_view name) {
  if constexpr (std::is_class_v<T>) {
    detail::AddProperty<T, PROPERTY>(&properties_, name);
  }
}

template <typename T>
inline NativeType<T>::NativeType(std::string_view name) : Type(name, typeid(T), nullptr) {
}

template <typename T>
Value::Value(T&& value) : type_(Type::Get<T>()), data_(std::move(value)) {}

template <typename T>
T& Value::Get() {
  assert(type_ == Type::Get<T>());
  return std::any_cast<T&>(data_);
}

template <typename T>
T Value::Get() const {
  return std::any_cast<std::remove_cvref_t<T>>(data_);
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

}  // namespace ovis

