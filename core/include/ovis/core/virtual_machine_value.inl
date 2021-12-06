namespace ovis {
namespace vm {


template <ReferenceType T>
Value Value::Create(T& value) {
  assert(Type::GetId<T>());
  return Value(*Type::GetId<T>(), safe_ptr(&value), true);
}

template <ReferenceType T>
Value Value::Create(T* value) {
  assert(Type::GetId<T>());
  return Value(*Type::GetId<T>(), safe_ptr(value), true);
}

template <ValueType T>
Value Value::Create(T&& value) {
  assert(Type::GetId<T>());
  return Value(*Type::GetId<T>(), std::forward<T>(value), false);
}

template <ValueType T>
Value Value::Create(T* value) {
  assert(Type::GetId<T>());
  return Value(*Type::GetId<T>(), *value, false);
}

inline Value Value::Create(const Value& value) {
  return value;
}

template <ReferenceType T>
Value Value::CreateView(T& value) {
  return Create(value);
}

template <ReferenceType T>
Value Value::CreateView(T* value) {
  return Create(value);
}

template <ValueType T>
Value Value::CreateView(T& value) {
  return Value(Type::Get<T>(), &value, true);
}

template <ValueType T>
Value Value::CreateView(T* value) {
  return Value(Type::Get<T>(), value, true);
}

inline Value Value::CreateView(Value& value) {
  assert(value.is_view() && "Cannot be implemented yet");
  return Value(value.type_id(), value.data_, value.is_view_);
}

template <typename T>
Value Value::CreateView(T&& value, std::shared_ptr<Type> actual_type) {
  return Value::CreateView(std::forward<T>(value)).CastToDerived(actual_type);
}

template <typename T>
Value Value::CreateViewIfPossible(T&& value) {
  if constexpr (
      std::is_same_v<T, Value> ||
      (
        std::is_reference_v<T> &&
        std::is_same_v<std::remove_reference_t<T>, Value> &&
        std::is_const_v<std::remove_reference_t<T>>
      )
  ) {
    return std::forward<T>(value);
  } else if constexpr (
    (
      std::is_reference_v<T> &&
      std::is_same_v<std::remove_reference_t<T>, Value> &&
      !std::is_const_v<std::remove_reference_t<T>>
    ) || (
      std::is_pointer_v<T> &&
      std::is_same_v<std::remove_pointer_t<T>, Value> &&
      !std::is_const_v<std::remove_pointer_t<T>>
    )
  ) {
    assert(false);
    return None();
  } else if constexpr (
      std::is_pointer_v<T> &&
      std::is_same_v<std::remove_pointer_t<T>, Value> && 
      std::is_const_v<std::remove_pointer_t<T>>
  ) {
    return Value(*value);
  } else if constexpr (
      is_reference_type_v<T> || 
      (std::is_pointer_v<T> && !std::is_const_v<std::remove_pointer_t<T>>) ||
      (std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>)) {
    return CreateView(std::forward<T>(value));
  } else {
    return Value::Create(std::forward<T>(value));
  }
}

template <ReferenceType T>
T& Value::Get() {
  if (type_id_ == Type::GetId<T>()) {
    return *std::any_cast<safe_ptr<T>>(data_).get();
  } else {
    return CastToBase(Type::Get<T>()).template Get<T>();
  }
}

template <PointerToReferenceType T>
T Value::Get() {
  using ReferenceType = std::remove_pointer_t<T>;
  if (type_id_ == Type::GetId<ReferenceType>()) [[likely]] {
    return std::any_cast<safe_ptr<ReferenceType>>(data_).get();
  } else if (type_id_ == Type::NONE_ID) {
    return nullptr;
  } else {
    return CastToBase(Type::Get<ReferenceType>()).template Get<T>();
  }
}

template <ValueType T>
std::remove_cvref_t<T>& Value::Get() {
  if constexpr (std::is_same_v<std::remove_cvref_t<T>, Value>) {
    return *this;
  } else {
    if (type_id() == Type::GetId<T>()) [[likely]] {
      return is_view_ ? *std::any_cast<std::remove_cvref_t<T>*>(data_) : std::any_cast<std::remove_cvref_t<T>&>(data_);
    } else if (const auto requested_type = Type::Get<T>(); Type::Get(type_id())->IsDerivedFrom(requested_type)) {
      Value base_type_value = CastToBase(requested_type);
      return base_type_value.Get<T>();
    } else {
      assert(false && "Invalid type requested");
      throw std::runtime_error("Invalid type requested");
    }
  }
}

template <PointerToValueType T>
T Value::Get() {
  return &Get<std::remove_pointer_t<T>>();
}

template <typename T>
inline void Value::SetProperty(std::string_view property_name, T&& property_value) {
  if constexpr (std::is_same_v<Value, T>) {
    const auto value_type = Type::Get(type_id());
    for (const auto& property : value_type->properties_) {
      if (property.name == property_name) {
        assert(property.type_id == type_id());
        assert(property.setter);
        property.setter(this, property_value);
        return;
      }
    }
    assert(false);
  } else {
    SetProperty(property_name, Value::CreateViewIfPossible(std::forward<T>(property_value)));
  }
}

template <typename T>
inline T Value::GetProperty(std::string_view property_name) {
  if constexpr (std::is_same_v<Value, T>) {
    const auto value_type = Type::Get(type_id());
    for (const auto& property : value_type->properties_) {
      if (property.name == property_name) {
        return property.getter(*this);
      }
    }
    assert(false);
    return None();
  } else {
    return GetProperty(property_name).Get<T>();
  }
}

inline json Value::Serialize() const {
  const auto value_type = Type::Get(type_id());
  if (value_type && value_type->serialize_function()) {
    return value_type->serialize_function()(*this);
  } else {
    return json();
  }
}

inline Value Value::CastToBase(std::shared_ptr<Type> target_type) {
  assert(target_type);
  const auto value_type = Type::Get(type_id());
  assert(value_type->IsDerivedFrom(target_type));
  if (target_type == value_type) {
    // TODO: create view
    return *this;
  } else {
    auto to_base = value_type->to_base_;
    if (to_base) {
      return to_base(*this).CastToBase(target_type);
    } else {
      return Value::None();
    }
  }
}

inline Value Value::CastToDerived(std::shared_ptr<Type> target_type) {
  assert(target_type);
  const auto value_type = Type::Get(type_id());
  assert(target_type->IsDerivedFrom(value_type));
  if (target_type == value_type) {
    // TODO: create view
    return *this;
  } else {
    std::vector<std::shared_ptr<Type>> types;
    for (std::shared_ptr<Type> current_type = target_type; current_type != value_type;
         current_type = current_type->parent_.lock()) {
      types.push_back(current_type);
    }
    Value value = Value::CreateView(*this);
    for (const std::shared_ptr<Type>& type : ReverseRange(types)) {
      assert(type->from_base_);
      value = type->from_base_(value);
    }
    return value;
  }
}

}
}

