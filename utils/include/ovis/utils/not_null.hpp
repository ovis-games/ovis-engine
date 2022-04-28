#pragma once

#include <cstddef>
#include <cassert>
#include <type_traits>

namespace ovis {

template <typename T> class NotNull;

template <typename T>
class NotNull<T*> {
public:
  NotNull() = delete;
  NotNull(std::nullptr_t) = delete;
  NotNull(T* pointer) : pointer_(pointer) { assert(pointer); }

  template <typename U> requires(std::is_base_of_v<T, U>)
  NotNull(NotNull<U*> other) : pointer_(other.pointer_) {}

  NotNull<T*>& operator=(std::nullptr_t) = delete;
  NotNull<T*>& operator=(T* pointer) {
    assert(pointer);
    pointer_ = pointer;
  }

  template <typename U> requires(std::is_base_of_v<T, U>)
  NotNull<T*>& operator=(NotNull<U*> other) {
    pointer_ = other.pointer_;
  }

  T* operator->() {
    return pointer_;
  }

  const T* operator->() const {
    return pointer_;
  }

  T& operator*() {
    return *pointer_;
  }

  const T& operator*() const {
    return *pointer_;
  }

 private:
  T* pointer_;
};

}  // namespace ovis
