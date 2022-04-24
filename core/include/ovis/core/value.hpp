#pragma once

#include <cstring>
#include <memory>
#include <string_view>
#include <type_traits>

#include <ovis/core/type.hpp>
#include <ovis/core/value_storage.hpp>

namespace ovis {

class Value {
 public:
  Value() : type_() {}
  ~Value();

  Value(const Value& other);
  Value& operator=(const Value&);

  template <typename T> T& as() { return storage_.as<T>(); }
  template <typename T> const T& as() const { return storage_.as<T>(); }
  const std::shared_ptr<Type>& type() const { return type_; }

  void Reset();

  template <typename T> Result<> SetProperty(std::string_view name, T&& value);
  template <typename T> Result<T> GetProperty(std::string_view name);

  template <typename T> static Value Create(T&& native_value);
  static Result<Value> Construct(std::shared_ptr<Type> type);

 private:
  ValueStorage storage_;
  std::shared_ptr<Type> type_;
};

}

// Implementation
// TODO:
//   * Refactor duplicated code!
//   * Reuse allocated storage
#include <ovis/core/function.hpp>

namespace ovis {

inline void Value::Reset() {
  type_.reset();
  storage_.reset();
}

template <typename T>
Result<> Value::SetProperty(std::string_view name, T&& value) {
  const auto property = type()->GetProperty(name);
  if (!property) {
    return Error("Type {} does not have a property with the name {}.", type()->name(), name);
  }

  const auto& property_type = property->type;
  assert(property_type != nullptr);
  if (property_type->id() != Type::GetId<T>()) {
    return Error("Invalid type for property {} of {}, expected `{}` got `{}`", name, type()->name(),
                 property_type->name(), Type::Get<T>()->name());
  }

  if (property->access.index() == 0) {
    const auto primitive_access = std::get<TypePropertyDescription::PrimitiveAccess>(property->access);
    auto property_pointer = static_cast<std::byte*>(storage_.value_pointer()) + primitive_access.offset;
    if (property_type->trivially_copy_assignable()) {
      std::memcpy(property_pointer, &value, sizeof(T));
    } else {
      OVIS_CHECK_RESULT(property_type->copy_assign_function()->Call<void>(property_pointer, &value));
    }
  } else {
    const auto function_access = std::get<TypePropertyDescription::FunctionAccess>(property->access);
    auto set_result = function_access.setter->Call<void>(storage_.value_pointer(), std::forward<T>(value));
    OVIS_CHECK_RESULT(set_result);
  }

  return Success;
}

template <typename T>
Result<T> Value::GetProperty(std::string_view name) {

}

template <typename T>
inline Value Value::Create(T&& native_value) {
  Value value;
  value.type_ = Type::Get<std::remove_cvref_t<T>>();
  value.storage_.reset(std::forward<T>(native_value));
  return value;
}

}  // namespace ovis
