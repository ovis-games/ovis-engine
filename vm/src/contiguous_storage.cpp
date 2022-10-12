#include "ovis/vm/contiguous_storage.hpp"

namespace ovis {

ContiguousStorage::ContiguousStorage(TypeMemoryLayout memory_layout)
    : memory_layout_(memory_layout), data_(nullptr), capacity_(0) {
  assert(memory_layout.is_constructible);
  assert(memory_layout.is_copyable);
}

ContiguousStorage::ContiguousStorage(TypeMemoryLayout memory_layout, SizeType capacity)
    : memory_layout_(memory_layout),
      data_(aligned_alloc(memory_layout_.alignment_in_bytes, capacity * memory_layout.size_in_bytes)),
      capacity_(capacity) {
  assert(memory_layout.is_constructible);
  assert(memory_layout.is_copyable);
  assert(data_);
}

ContiguousStorage::~ContiguousStorage() {
  free(data_);
}

}  // namespace ovis
