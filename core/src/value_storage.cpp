#include <ovis/core/execution_context.hpp>
#include <ovis/core/value_storage.hpp>

namespace ovis {

void ValueStorage::Reset(NotNull<ExecutionContext*> execution_context) {
  const bool is_storage_allocated = has_allocated_storage();
  auto destruct = destruct_function();
  if (destruct) {
    if (execution_context->Call<void>(destruct, value_pointer())) {
      SetDestructFunction(FunctionHandle::Null());
    } else {
      // No Result<> type here as we do not know how to recover from failed destructors anyway
      throw std::runtime_error("Failed to destruct object");
    }
  }
  if (is_storage_allocated) {
    Deallocate();
    SetAllocatedStorageFlag(false);
  }
#ifndef NDEBUG
  native_type_id_ = TypeOf<void>;
#endif
}

void ValueStorage::ResetTrivial() {
  assert(!has_allocated_storage());
  assert(!destruct_function());
#ifndef NDEBUG
  native_type_id_ = TypeOf<void>;
#endif
}

void* ValueStorage::AllocateIfNecessary(std::size_t alignment, std::size_t size) {
  assert(!has_allocated_storage());
  assert(!destruct_function());
  if (IsTypeStoredInline(alignment, size)) {
    return data();
  } else {
    Allocate(alignment, size);
    return allocated_storage_pointer();
  }
}

void ValueStorage::Allocate(std::size_t alignment, std::size_t size) {
  new (&data_) void*(aligned_alloc(alignment, size));
  SetAllocatedStorageFlag(true);
}

void ValueStorage::Deallocate() {
  assert(has_allocated_storage());
  std::free(allocated_storage_pointer());
  SetAllocatedStorageFlag(false);
}

void ValueStorage::SetDestructFunction(FunctionHandle destructor) {
  assert((destructor.integer & 1) == 0);
  destruct_function_and_flags = destructor.integer | flags_.allocated_storage;
}

FunctionHandle ValueStorage::destruct_function() const {
  return { .integer = destruct_function_and_flags & ~1 };
}

void ValueStorage::CopyTrivially(ValueStorage* destination, const ValueStorage* source) {
  assert(!destination->has_allocated_storage());
  assert(!source->has_allocated_storage());
  std::memcpy(&destination->data_, &source->data_, SIZE);
  destination->destruct_function_and_flags = source->destruct_function_and_flags;
#ifndef NDEBUG
  destination->native_type_id_ = source->native_type_id_;
#endif
}

}  // namespace ovis
