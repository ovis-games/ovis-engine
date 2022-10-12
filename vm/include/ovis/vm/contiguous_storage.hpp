#pragma once

#include <cassert>

#include "ovis/utils/memory.hpp"
#include "ovis/utils/result.hpp"
#include "ovis/vm/type.hpp"

namespace ovis {

// This "container" manages the contigous storage for n elements. It provides
// functionality for constructing, destructing and copying to the objects in the storage.
// However, does not store information on which elements are currently constructed. This
// has to be managed elsewhere. For that reason it also does not support copying, moving
// or changing the capacity. See List or ComponentStorage for possible use cases.
class ContiguousStorage {
  friend void swap(ContiguousStorage&, ContiguousStorage&);

 public:
  // 32 bit because the number type in the VM is a double and 32 bit
  // can be represented without less in contrast to 64 bit.
  using SizeType = std::uint32_t;

  ContiguousStorage(TypeMemoryLayout memory_layout);
  ContiguousStorage(TypeMemoryLayout memory_layout, SizeType capacity);

  ContiguousStorage(const ContiguousStorage& other) = delete;
  ContiguousStorage(ContiguousStorage&& other) = delete;

  ~ContiguousStorage();

  ContiguousStorage& operator=(const ContiguousStorage& other) = delete;
  ContiguousStorage& operator=(ContiguousStorage&& other) = delete;

  const TypeMemoryLayout& memory_layout() const { return memory_layout_; }
  SizeType capacity() const { return capacity_; }
  void* data() { return data_; }
  const void* data() const { return data_; }

  void* operator[](SizeType index) {
    return GetElementAddress(index);
  }

  const void* operator[](SizeType index) const {
    return GetElementAddress(index);
  }

  Result<> Construct(SizeType index) {
    return memory_layout_.Construct(GetElementAddress(index));
  }

  void Destruct(SizeType index) {
    memory_layout_.Destruct(GetElementAddress(index));
  }

  Result<> CopyTo(SizeType index, const void* source) {
    return memory_layout_.Copy(GetElementAddress(index), source);
  }

  Result<> ConstructRange(SizeType index, SizeType count) {
    assert(index + count < capacity());
    return memory_layout_.ConstructN(GetElementAddress(index), count);
  }

  void DestructRange(SizeType index, SizeType count) {
    assert(index + count < capacity());
    memory_layout_.DestructN(GetElementAddress(index), count);
  }

  Result<> CopyToRange(SizeType index, SizeType count, const void* source) {
    assert(index + count < capacity());
    return memory_layout_.CopyN(GetElementAddress(index), source, count);
  }

 private:
  TypeMemoryLayout memory_layout_;
  SizeType capacity_;
  void* data_;

  void* GetElementAddress(SizeType index) {
    assert(index < capacity());
    return OffsetAddress(data_, index * memory_layout_.size_in_bytes);
  }

  const void* GetElementAddress(SizeType index) const {
    assert(index < capacity());
    return OffsetAddress(data_, index * memory_layout_.size_in_bytes);
  }
};

inline void swap(ContiguousStorage& a, ContiguousStorage& b) {
  using std::swap;
  swap(a.memory_layout_, b.memory_layout_);
  swap(a.data_, b.data_);
  swap(a.capacity_, b.capacity_);
}

}  // namespace ovis
