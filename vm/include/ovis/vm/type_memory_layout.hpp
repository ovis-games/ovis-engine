#pragma once

#include <memory>

#include "ovis/utils/native_type_id.hpp"
#include "ovis/utils/result.hpp"

namespace ovis {

class Function;
class ExecutionContext;

// This structure defined the memory information of a type.
struct TypeMemoryLayout {
  NativeTypeId native_type_id;

  std::uint32_t is_constructible : 1;
  std::uint32_t is_copyable : 1;

  std::uintptr_t alignment_in_bytes;
  std::uintptr_t size_in_bytes;

  // If is_copy_constructible is true, this member must point to a function that constructs the type.
  std::shared_ptr<Function> construct;
  // If is_copyable is true, this member may point to a function that copies the type. Otherwise it is assumed the type
  // is trivially copyable.
  std::shared_ptr<Function> copy;
  // This member may point to a function that destructs the type. Otherwise, it is assumed the type is trivially
  // destructible.
  std::shared_ptr<Function> destruct;

  // Constructs a single object
  Result<> Construct(ExecutionContext* execution_context, void* memory) const;
  Result<> Construct(void* memory) const;

  // Constructs count objects of the type for a given memory.
  // If an error occurs during construction, all constructed objects will be destructed.
  Result<> ConstructN(void* memory, std::size_t count) const;

  // Destructs an object. If an error occurs the program will be terminated.
  void Destruct(void* object) const;

  // Destructs count objects. If an error occurs it will terminate the program.
  void DestructN(void* objects, std::size_t count) const;

  // Copies a single object from source to destination
  Result<> Copy(void* destination, const void* source) const;

  // Copies count objects from source to destination. If an error occurs during copying the objects may be be partially copied.
  // Source and destination may not overlap (because of memcpy).
  Result<> CopyN(void* destination, const void* source, std::size_t count) const;
};
bool operator==(const TypeMemoryLayout& lhs, const TypeMemoryLayout& rhs);
bool operator!=(const TypeMemoryLayout& lhs, const TypeMemoryLayout& rhs);

}  // namespace ovis
