#pragma once

#include <cstdint>

#include "ovis/utils/not_null.hpp"
#include "ovis/utils/result.hpp"
#include "ovis/vm/contiguous_storage.hpp"
#include "ovis/vm/type.hpp"
#include "ovis/vm/type_id.hpp"
#include "ovis/vm/value.hpp"
#include "ovis/vm/virtual_machine.hpp"

namespace ovis {

class List {
 public:
  // 32 bit because the number type in the VM is a double and 32 bit
  // can be represented without less in contrast to 64 bit.
  using SizeType = std::uint32_t;

  // template <bool is_const> struct ValueRef;
  // template <> struct ValueRef<true> {
  //   const void* pointer

  // };

  List(NotNull<Type*> element_type);
  List(TypeId element_type, NotNull<VirtualMachine*> virtual_machine);

  List(const List& other);
  List(List&& other);

  ~List();

  List& operator=(const List& other);
  List& operator=(List&& other);

  TypeId element_type() const { return element_type_; }
  SizeType size() const { return size_; }
  SizeType capacity() const { return storage_.capacity(); }
  TypeMemoryLayout memory_layout() const { return storage_.memory_layout(); }

  Result<> Reserve(SizeType new_capacity);
  Result<> Resize(SizeType new_size);

  template <typename T>
  Result<> Add(T&& value) {
    // TODO: assert right type
    return AddInternal(&value);
  }

  Result<> Add(const Value& value);
  Result<> Remove(SizeType index);

  template <typename T>
  const T& Get(SizeType index) const {
    // TODO: assert right type
    return *reinterpret_cast<const T*>(storage_[index]);
  }

  template <typename T>
  void Set(SizeType index, T value) {
    // TODO: assert right type
    *reinterpret_cast<T*>(storage_[index]) = value;
  }

 private:
  TypeId element_type_;
  ContiguousStorage storage_;
  SizeType size_ = 0;

  Result<> AddInternal(const void* source);
};

}  // namespace ovis
