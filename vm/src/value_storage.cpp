#include <ovis/vm/execution_context.hpp>
#include <ovis/vm/value_storage.hpp>
#include <ovis/vm/type.hpp>

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
    return Allocate(alignment, size);
  }
}

void* ValueStorage::Allocate(std::size_t alignment, std::size_t size) {
  new (&data_) void*(aligned_alloc(alignment, size));
  SetAllocatedStorageFlag(true);
  return allocated_storage_pointer();
}

void ValueStorage::Deallocate() {
  assert(has_allocated_storage());
  std::free(allocated_storage_pointer());
  SetAllocatedStorageFlag(false);
}

void ValueStorage::SetDestructFunction(FunctionHandle destructor) {
  assert(destructor.zero == 0);
  const bool allocated_storage = has_allocated_storage();
  destruct_function_and_flags = destructor.integer;
  flags_.allocated_storage = allocated_storage;
}

FunctionHandle ValueStorage::destruct_function() const {
  FunctionHandle handle { .integer = destruct_function_and_flags };
  handle.zero = 0;
  return handle;
}

Result<> ValueStorage::Construct(NotNull<ExecutionContext*> execution_context, const TypeMemoryLayout& layout) {
  assert(layout.is_constructible);
  assert(layout.construct);

  auto* value_pointer = AllocateIfNecessary(layout.alignment_in_bytes, layout.size_in_bytes);
  assert(value_pointer);

  const auto construct_result = execution_context->Call<void>(layout.construct->handle(), value_pointer);
  OVIS_CHECK_RESULT(construct_result);
  
  if (layout.destruct) {
    SetDestructFunction(layout.destruct->handle());
  }
#ifndef NDEBUG
  native_type_id_ = layout.native_type_id;
#endif

  return Success;
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

Result<> ValueStorage::Copy(NotNull<ExecutionContext*> execution_context, const TypeMemoryLayout& layout,
                            NotNull<ValueStorage*> destination, NotNull<const ValueStorage*> source) {
  assert(destination->has_allocated_storage() == source->has_allocated_storage());
  assert(destination->destruct_function() == source->destruct_function());
  assert(destination->destruct_function() == (layout.destruct ? layout.destruct->handle() : FunctionHandle::Null()));
  assert(source->destruct_function() == (layout.destruct ? layout.destruct->handle() : FunctionHandle::Null()));
  assert(layout.is_copyable);

  void* destination_value_pointer = destination->value_pointer();
  const void* source_value_pointer = source->value_pointer();
  if (layout.destruct) {
    OVIS_CHECK_RESULT(execution_context->Call<void>(layout.copy->handle(), destination_value_pointer, source_value_pointer));
  } else {
    std::memcpy(destination_value_pointer, source_value_pointer, layout.size_in_bytes);
  }

  return Success;
}

}  // namespace ovis
