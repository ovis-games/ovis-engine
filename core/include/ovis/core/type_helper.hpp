#pragma once

#include <cassert>
#include <cstdint>  // for std::uintptr_t
#include <utility>  // for std::move

namespace ovis {
namespace type_helper {

template <typename T>
void DefaultConstruct(void* destination) {
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignof(T) == 0);
  new (destination) T();
}

template <typename T>
void CopyConstruct(void* destination, const void * source) {
  assert(reinterpret_cast<std::uintptr_t>(source) % alignof(T) == 0);
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignof(T) == 0);
  new (destination) T(*reinterpret_cast<const T*>(source));
}

template <typename T>
void MoveConstruct(void* destination, const void * source) {
  assert(reinterpret_cast<std::uintptr_t>(source) % alignof(T) == 0);
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignof(T) == 0);
  new (destination) T(std::move(*reinterpret_cast<T*>(source)));
}

template <typename T>
void CopyAssign(void* destination, const void * source) {
  assert(reinterpret_cast<std::uintptr_t>(source) % alignof(T) == 0);
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignof(T) == 0);
  *reinterpret_cast<T*>(destination) = *reinterpret_cast<const T*>(source);
}

template <typename T>
void MoveAssign(void* destination, const void * source) {
  assert(reinterpret_cast<std::uintptr_t>(source) % alignof(T) == 0);
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignof(T) == 0);
  *reinterpret_cast<T*>(destination) = std::move(*reinterpret_cast<T*>(source));
}

template <typename T>
void Destruct(void* value) {
  reinterpret_cast<T*>(value)->~T();
}

}  // namespace type_helper
}  // namespace ovis
