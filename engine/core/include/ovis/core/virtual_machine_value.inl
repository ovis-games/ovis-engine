namespace ovis {
namespace vm {

template <ReferenceType T>
Value::Value(T& value) : type_(Type::Get<T>()), data_(safe_ptr<T>(&value)) {}

template <ReferenceType T>
Value::Value(T* value) : type_(Type::Get<T>()), data_(safe_ptr<T>(value)) {}

template <NonReferenceType T>
Value::Value(T&& value) : type_(Type::Get<T>()), data_(std::forward<T>(value)) {}

template <PointerToReferenceType T>
T Value::Get() {
  using ReferenceType = std::remove_pointer_t<T>;
  assert(type_ == Type::Get<ReferenceType>());
  return std::any_cast<safe_ptr<ReferenceType>>(data_).get();
}

// template <PointerToReferenceType T> T Get() const;

template <NonReferenceType T>
std::remove_cvref_t<T>& Value::Get() {
  if constexpr (std::is_same_v<std::remove_cvref_t<T>, Value>) {
    return *this;
  } else {
    const auto requested_type = Type::Get<T>();
    if (type_ == Type::Get<T>()) {
      return std::any_cast<std::remove_cvref_t<T>&>(data_);
    } else if (type_->IsDerivedFrom(requested_type)) {
      Value base_type_value = CastToBase(*this, requested_type);
      return base_type_value.Get<T>();
    } else {
      assert(false && "Invalid type requested");
    }
  }
}

template <NonReferenceType T>
const std::remove_cvref_t<T>& Value::Get() const {
  if constexpr (std::is_same_v<std::remove_cvref_t<T>, Value>) {
    return *this;
  } else {
    const auto requested_type = Type::Get<T>();
    if (type_ == Type::Get<T>()) {
      return std::any_cast<const std::remove_cvref_t<T>&>(data_);
    } else if (type_->IsDerivedFrom(requested_type)) {
      Value base_type_value = CastToBase(*this, requested_type);
      return base_type_value.Get<T>();
    } else {
      assert(false && "Invalid type requested");
    }
  }
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

inline Value Value::CastToBase(const Value& value, safe_ptr<Type> target_type) {
  assert(value.type()->IsDerivedFrom(target_type));
  if (target_type == value.type()) {
    return value;
  } else {
    return CastToBase(value.type()->to_base_(value), target_type);
  }
}

}
}

