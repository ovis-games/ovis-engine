#include "ovis/vm/list.hpp"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include "ovis/utils/not_null.hpp"

namespace ovis {

namespace {

void* OffsetAddress(void* address, std::uintptr_t offset) {
  return reinterpret_cast<uint8_t*>(address) + offset;
}

}  // namespace

List::List(NotNull<Type*> type) : element_type_(type->id()), memory_layout_(type->memory_layout()) {
  assert(memory_layout_.is_constructible);
  assert(memory_layout_.is_copyable);
}

List::List(TypeId element_type, NotNull<VirtualMachine*> virtual_machine)
    : List(virtual_machine->GetType(element_type)) {}

void List::Reserve(SizeType new_capacity) {
  if (new_capacity <= capacity_) {
    return;
  }

  void* new_data = aligned_alloc(memory_layout_.alignment_in_bytes, new_capacity * memory_layout_.size_in_bytes);
  assert(new_data != nullptr);

  for (SizeType i = 0; i < size_; ++i) {
    const std::uintptr_t offset = i * memory_layout_.size_in_bytes;
    const auto result = memory_layout_.copy->Call(OffsetAddress(new_data, offset), OffsetAddress(data_, offset));
    assert(result);  // TODO: fail appropriately
  }

  std::swap(new_data, data_);
  capacity_ = new_capacity;

  // Destruct old elements
  for (SizeType i = 0; i < size_; ++i) {
    const std::uintptr_t offset = i * memory_layout_.size_in_bytes;
    const auto result = memory_layout_.destruct->Call(OffsetAddress(new_data, offset));
    assert(result);  // TODO: fail appropriately
  }
}

void List::Resize(SizeType new_size) {
  if (size() < new_size) {
    Reserve(new_size);
  } else if (size() > new_size) {
  }
}

}  // namespace ovis
