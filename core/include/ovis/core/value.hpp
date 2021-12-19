#pragma once

#include <memory>
#include <type_traits>

#include <ovis/core/type.hpp>
#include <ovis/core/value_storage.hpp>

namespace ovis {

class Value {
 public:
  Value() : type_(Type::NONE_ID) {}
  // template <typename T>
  // Value(T&& value) : type_(Type::GetId<T>()), storage_(std::make_unique<ValueStorage>(std::forward<T>(value))) {}
  Value(const Value&) = default;
  Value(Value&&) = default;
  Value& operator=(const Value&) = default;
  Value& operator=(Value&&) = default;

 private:
  Type::Id type_;
  std::shared_ptr<ValueStorage> storage_;
};
static_assert(std::is_move_constructible_v<Value>);
static_assert(std::is_move_assignable_v<Value>);

}  // namespace ovis
