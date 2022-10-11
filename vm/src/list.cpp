#include "ovis/vm/list.hpp"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "ovis/utils/memory.hpp"
#include "ovis/utils/not_null.hpp"

namespace ovis {

List::List(NotNull<Type*> type) : element_type_(type->id()), memory_layout_(type->memory_layout()) {
  assert(memory_layout_.is_constructible);
  assert(memory_layout_.is_copyable);
}

List::List(TypeId element_type, NotNull<VirtualMachine*> virtual_machine)
    : List(virtual_machine->GetType(element_type)) {}

List::List(const List& other) : element_type_(other.element_type()), memory_layout_(other.memory_layout_) {
  Resize(other.size());
  memory_layout_.CopyN(data_, other.data_, other.size());
}

List::List(List&& other)
    : element_type_(other.element_type()),
      memory_layout_(other.memory_layout_),
      size_(other.size_),
      capacity_(other.capacity_),
      data_(other.data_) {
  other.data_ = nullptr;
  other.size_ = 0;
  other.capacity_ = 0;
}

List::~List() {
  memory_layout_.DestructN(data_, size());
  free(data_);
}

List& List::operator=(const List& other) {
  assert(other.element_type() == element_type());
  assert(other.memory_layout_ == memory_layout_);

  Resize(0);  // First resize to zero, so that if the second resize call will reallocate the memory,
              // no elements have to be copied.
  Resize(other.size());
  memory_layout_.CopyN(data_, other.data_, size());

  return *this;
}

List& List::operator=(List&& other) {
  assert(other.element_type() == element_type());
  assert(other.memory_layout_ == memory_layout_);

  // Just swapping because its fast. Should we do something else here?
  std::swap(other.data_, data_);
  std::swap(other.size_, size_);
  std::swap(other.capacity_, capacity_);

  return *this;
}

void List::Reserve(SizeType new_capacity) {
  if (new_capacity <= capacity_) {
    return;
  }

  void* new_data = aligned_alloc(memory_layout_.alignment_in_bytes, new_capacity * memory_layout_.size_in_bytes);
  assert(new_data != nullptr);

  memory_layout_.ConstructN(new_data, size());
  memory_layout_.CopyN(new_data, data_, size());
  memory_layout_.DestructN(data_, size());
  free(data_);

  data_ = new_data;
  capacity_ = new_capacity;
}

void List::Resize(SizeType new_size) {
  if (size() < new_size) {
    Reserve(new_size);
    memory_layout_.ConstructN(GetElementAddress(size()), new_size - size());
  } else if (size() > new_size) {
    memory_layout_.DestructN(GetElementAddress(new_size), size() - new_size);
  }
  size_ = new_size;
}

Result<> List::Add(const Value& value) {
  return AddInternal(value.GetValuePointer());
}

Result<> List::Remove(SizeType index) {
  if (index >= size()) {
    return Error("Index out of bounds");
  }
  for (SizeType i = index; i < size() - 1; ++i) {
    OVIS_CHECK_RESULT(memory_layout_.Copy(GetElementAddress(i), GetElementAddress(i + 1)));
  }
  memory_layout_.Destruct(GetElementAddress(size() - 1));
  --size_;
  return Success;
}

void* List::GetElementAddress(SizeType index) {
  assert(index < capacity());
  return OffsetAddress(data_, index * memory_layout_.size_in_bytes);
}

const void* List::GetElementAddress(SizeType index) const {
  assert(index < capacity());
  return OffsetAddress(data_, index * memory_layout_.size_in_bytes);
}

Result<> List::AddInternal(const void* source) {
  if (size() == capacity()) {
    const auto new_capacity = size() + size() / 2 + 1; // +1 to handle 0 and 1 case.
    Reserve(new_capacity);
  }

  auto new_element_address = GetElementAddress(size());
  OVIS_CHECK_RESULT(memory_layout_.Construct(new_element_address));
  if (auto result = memory_layout_.Copy(new_element_address, source); !result) {
    memory_layout_.Destruct(new_element_address);
    return std::move(result);
  }
  ++size_;
  return Success;
}

}  // namespace ovis
