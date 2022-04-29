#pragma once

#include <cstddef>
#include <cassert>
#include <type_traits>

namespace ovis {

template <typename T> class NotNull;

template <typename T>
class NotNull<T*> {
  template <typename U> friend bool operator==(NotNull<U*> lhs, NotNull<U*> rhs);
  template <typename U> friend bool operator==(U* lhs, NotNull<U*> rhs);
  template <typename U> friend bool operator==(NotNull<U*> lhs, U* rhs);

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

template <typename T>
bool operator==(NotNull<T*> lhs, NotNull<T*> rhs) {
  return lhs.pointer_ == rhs.pointer_;
}

template <typename T>
bool operator==(T* lhs, NotNull<T*> rhs) {
  return lhs == rhs.pointer_;
}

template <typename T>
bool operator==(NotNull<T*> lhs, T* rhs) {
  return lhs.pointer_ == rhs;
}

template <typename T>
bool operator!=(NotNull<T*> lhs, NotNull<T*> rhs) {
  return !(lhs == rhs);
}

template <typename T>
bool operator!=(T* lhs, NotNull<T*> rhs) {
  return !(lhs == rhs);
}

template <typename T>
bool operator!=(NotNull<T*> lhs, T* rhs) {
  return !(lhs == rhs);
}

}  // namespace ovis
