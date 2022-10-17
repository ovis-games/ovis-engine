#pragma once

#include <cstring>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

#include "ovis/utils/native_type_id.hpp"
#include "ovis/utils/not_null.hpp"
#include "ovis/utils/result.hpp"
#include "ovis/vm/type_id.hpp"
#include "ovis/vm/value_storage.hpp"

namespace ovis {

class Value {
  friend class VirtualMachine;

 public:
  Value(NotNull<VirtualMachine*> virtual_machine);
  Value(NotNull<VirtualMachine*> virtual_machine, TypeId type_id);
  Value(NotNull<Type*> type);
  ~Value();

  Value(const Value& other);
  Value& operator=(const Value&);

  NotNull<VirtualMachine*> virtual_machine() const { return virtual_machine_; }

  template <typename T> T& as() {
    assert(GetValuePointer());
    return *reinterpret_cast<T*>(GetValuePointer());
  }
  template <typename T> const T& as() const {
    assert(GetValuePointer());
    return *reinterpret_cast<const T*>(GetValuePointer());
  }

  const void* GetValuePointer() const;
  void* GetValuePointer();

  TypeId type_id() const { return type_id_; }
  Type* type() const;
  const TypeMemoryLayout* memory_layout() const;
  bool is_reference() const { return is_reference_; }
  bool has_value() const;

  Value CreateReference() const;

  void Reset();

  Result<> CopyTo(NotNull<Value*> other) const;
  Result<> CopyTo(NotNull<ExecutionContext*> execution_context, NotNull<Value*> other) const;
  Result<> CopyTo(NotNull<ValueStorage*> storage) const;
  Result<> CopyTo(NotNull<ExecutionContext*> execution_context, NotNull<ValueStorage*> storage) const;

  template <typename T> Result<> SetProperty(std::string_view name, T&& value);
  template <typename T> Result<> SetProperty(ExecutionContext* execution_context, std::string_view name, T&& value);
  template <typename T> Result<T> GetProperty(std::string_view name);
  template <typename T> Result<T> GetProperty(ExecutionContext* execution_context, std::string_view name);

 private:
  ValueStorage storage_;
  NotNull<VirtualMachine*> virtual_machine_;
  TypeId type_id_;
  bool is_reference_ : 1;

  // Type erased versions of the methods above. This allows mving the actual implementation to the source file
  // and thus we can avoid including additional headers reducing the dependency
  Result<> SetProperty(std::string_view name, NativeTypeId type_id, const void* value);
  Result<> SetProperty(ExecutionContext* execution_context, std::string_view name, NativeTypeId type_id, const void* value);
  Result<> GetProperty(std::string_view name, NativeTypeId type_id, void* value);
  Result<> GetProperty(ExecutionContext* execution_context, std::string_view name, NativeTypeId type_id, void* value);
};

}  // namespace ovis

// Implementation
// TODO:
//   * Refactor duplicated code!
//   * Reuse allocated storage
#include <ovis/vm/function.hpp>

namespace ovis {

template <typename T>
Result<> Value::SetProperty(std::string_view name, T&& value) {
  return SetProperty(name, TypeOf<T>, &value);
}

template <typename T>
Result<> Value::SetProperty(ExecutionContext* execution_context, std::string_view name, T&& value) {
  using Type = std::remove_cvref_t<T>;

}

template <typename T>
Result<T> Value::GetProperty(std::string_view name) {
  T value;
  OVIS_CHECK_RESULT(GetProperty(name, TypeOf<T>, &value));
  return value;
}

template <typename T>
Result<T> Value::GetProperty(ExecutionContext* execution_context, std::string_view name) {
  using Type = std::remove_cvref_t<T>;

}

}  // namespace ovis
