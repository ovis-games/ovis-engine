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

 protected:
  Type(std::string_view name, Type* parent = nullptr);
  Type(std::string_view name, const std::type_info& associated_type, Type* parent = nullptr);

  std::vector<Property> properties_;

 private:
  std::string name_; 
  std::optional<std::type_index> associated_type_;
  safe_ptr<Type> parent_;

  static std::vector<Type> types;
};

class Value {
 public:
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

}  // namespace ovis

