#include "ovis/vm/type_memory_layout.hpp"

#include "ovis/utils/memory.hpp"
#include "ovis/vm/function.hpp"
#include "ovis/vm/virtual_machine.hpp"

namespace ovis {

Result<> TypeMemoryLayout::ConstructN(void* memory, std::size_t count) const {
  assert(reinterpret_cast<std::uintptr_t>(memory) % alignment_in_bytes == 0);

  for (std::size_t i = 0; i < count; ++i) {
    const auto execution_context = construct->virtual_machine()->main_execution_context();
    const auto result = execution_context->Call(construct->handle(), OffsetAddress(memory, i * size_in_bytes));
    // Construction failed. Destruct all previously constructed objects.
    if (!result) {
      DestructN(memory, i);
      return result;
    }
  }

  return Success;
}

Result<> TypeMemoryLayout::Construct(void* memory) const {
  assert(memory != nullptr);
  assert(reinterpret_cast<std::uintptr_t>(memory) % alignment_in_bytes == 0);
  const auto execution_context = construct->virtual_machine()->main_execution_context();
  return execution_context->Call(construct->handle(), memory);
}

void TypeMemoryLayout::DestructN(void* objects, std::size_t count) const {
  assert(reinterpret_cast<std::uintptr_t>(objects) % alignment_in_bytes == 0);

  if (destruct) {
    for (std::size_t i = 0; i < count; ++i) {
      const auto execution_context = destruct->virtual_machine()->main_execution_context();
      const auto result = execution_context->Call(destruct->handle(), OffsetAddress(objects, i * size_in_bytes));
      assert(result);  // Destruction should never fail
    }
  }
}

void TypeMemoryLayout::Destruct(void* object) const {
  assert(object != nullptr);
  assert(reinterpret_cast<std::uintptr_t>(object) % alignment_in_bytes == 0);
  if (destruct) {
    const auto execution_context = destruct->virtual_machine()->main_execution_context();
    const auto result = execution_context->Call(destruct->handle(), object);
    assert(result);
  }
}

Result<> TypeMemoryLayout::CopyN(void* destination, const void* source, std::size_t count) const {
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignment_in_bytes == 0);
  assert(reinterpret_cast<std::uintptr_t>(source) % alignment_in_bytes == 0);

  if (copy) {
    const auto execution_context = copy->virtual_machine()->main_execution_context();
    for (std::size_t i = 0; i < count; ++i) {
      const std::uintptr_t offset = i * size_in_bytes;
      OVIS_CHECK_RESULT(
          execution_context->Call(copy->handle(), OffsetAddress(destination, offset), OffsetAddress(source, offset)));
    }
  } else if (count > 0) {
    // memcpy is undefined if source or destination is null, so only copy if there is actually something to copy
    assert(destination != nullptr);
    assert(source != nullptr);
    std::memcpy(destination, source, size_in_bytes * count);
  }

  return Success;
}

Result<> TypeMemoryLayout::Copy(void* destination, const void* source) const {
  assert(destination != nullptr);
  assert(source != nullptr);
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignment_in_bytes == 0);
  assert(reinterpret_cast<std::uintptr_t>(source) % alignment_in_bytes == 0);

  if (copy) {
    const auto execution_context = copy->virtual_machine()->main_execution_context();
    return execution_context->Call(copy->handle(), destination, source);
  } else {
    std::memcpy(destination, source, size_in_bytes);
    return Success;
  }
}

bool operator==(const TypeMemoryLayout& lhs, const TypeMemoryLayout& rhs) {
  return
    lhs.native_type_id == rhs.native_type_id && 
    lhs.is_constructible == rhs.is_constructible && 
    lhs.is_copyable == rhs.is_copyable && 
    lhs.alignment_in_bytes == rhs.alignment_in_bytes && 
    lhs.size_in_bytes == rhs.size_in_bytes && 
    lhs.construct->handle() == rhs.construct->handle() && 
    ((!lhs.copy && !rhs.copy) || (lhs.copy && rhs.copy && lhs.copy->handle() == rhs.copy->handle())) && 
    ((!lhs.destruct && !rhs.destruct) || (lhs.destruct && rhs.destruct && lhs.destruct->handle() == rhs.destruct->handle()));
}

bool operator!=(const TypeMemoryLayout& lhs, const TypeMemoryLayout& rhs) {
  return !(lhs == rhs);
}

}  // namespace ovis
