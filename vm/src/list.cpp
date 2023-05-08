#include "ovis/vm/list.hpp"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "ovis/utils/memory.hpp"
#include "ovis/utils/not_null.hpp"

namespace ovis {

List::List(NotNull<Type*> type) : element_type_(type->id()), storage_(type->memory_layout(), 0) {}

List::List(TypeId element_type, NotNull<VirtualMachine*> virtual_machine)
    : List(virtual_machine->GetType(element_type)) {}

List::List(const List& other)
    : element_type_(other.element_type()),
      storage_(other.storage_.memory_layout(), other.storage_.capacity()),
      size_(other.size_) {
  storage_.CopyToRange(0, size_, other.storage_[0]);
}

List::List(List&& other)
    : element_type_(other.element_type()),
      storage_(other.storage_.memory_layout()),
      size_(other.size_) {
  using std::swap;
  swap(storage_, other.storage_);
  other.size_ = 0;
}

List::~List() {
  storage_.DestructRange(0, size());
}

List& List::operator=(const List& other) {
  assert(other.element_type() == element_type());
  assert(other.storage_.memory_layout() == storage_.memory_layout());

  Resize(0);  // First resize to zero, so that if the second resize call will reallocate the memory,
              // no elements have to be copied.
  Resize(other.size());
  storage_.CopyToRange(0, size(), other.storage_[0]);

  return *this;
}

List& List::operator=(List&& other) {
  assert(other.element_type() == element_type());
  assert(other.storage_.memory_layout() == storage_.memory_layout());

  // Just swapping because its fast. Should we do something else here? - No
  using std::swap;
  swap(storage_, other.storage_);
  std::swap(other.size_, size_);

  return *this;
}

Result<> List::Reserve(SizeType new_capacity) {
  if (new_capacity <= storage_.capacity()) {
    return Success;
  }

  ContiguousStorage new_storage(memory_layout(), new_capacity);
  OVIS_CHECK_RESULT(new_storage.ConstructRange(0, size()));
  OVIS_CHECK_RESULT(new_storage.CopyToRange(0, size(), storage_.data()));
  swap(storage_, new_storage);
  return Success;
}

Result<> List::Resize(SizeType new_size) {
  if (size() < new_size) {
    OVIS_CHECK_RESULT(Reserve(new_size));
    OVIS_CHECK_RESULT(storage_.ConstructRange(size(), new_size - size()));
    // If construction fails the capacity grew but the size remains the same.
    // Should be fine.
  } else if (size() > new_size) {
    storage_.DestructRange(new_size, size() - new_size);
  }
  size_ = new_size;
  return Success;
}

Result<> List::Add(const Value& value) {
  if (value.type_id() != element_type()) {
    return Error("Invalid type.");
  }
  return AddInternal(value.GetValuePointer());
}

Result<> List::Remove(SizeType index) {
  if (index >= size()) {
    return Error("Index out of bounds");
  }
  for (SizeType i = index; i < size() - 1; ++i) {
    OVIS_CHECK_RESULT(storage_.CopyTo(i, storage_[i + 1]));
    // If this fails, parts of the elements have been overwritten
  }
  storage_.Destruct(size() - 1);
  --size_;
  return Success;
}

Result<> List::AddInternal(const void* source) {
  if (size() == capacity()) {
    const auto new_capacity = size() + size() / 2 + 1; // +1 to handle 0 and 1 case.
    Reserve(new_capacity);
  }

  OVIS_CHECK_RESULT(storage_.Construct(size()));
  if (auto result = storage_.CopyTo(size(), source); !result) {
    storage_.Destruct(size());
    return std::move(result);
  }
  ++size_;
  return Success;
}

}  // namespace ovis
