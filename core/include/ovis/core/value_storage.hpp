#pragma once

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <utility>

#include <ovis/utils/type_id.hpp>
#include <ovis/core/function_handle.hpp>

namespace ovis {

class alignas(16) ValueStorage final {
 public:
  constexpr static std::size_t ALIGNMENT = 8;
  constexpr static std::size_t SIZE = 8;

  template <typename T>
  static constexpr bool stored_inline = alignof(T) <= ALIGNMENT && sizeof(T) <= SIZE;

  // We will fit pointers and doubles in the value field, so they should fit and properly aligned
  static_assert(stored_inline<void*>);
  static_assert(stored_inline<double>);

  // Constructs an empty value storage
  ValueStorage() : destruct_function_and_flags(0) {}

  // Stores a value of type T in the storage. See reset(T&&)
  template <typename T> ValueStorage(T&& value) : ValueStorage() { reset(std::forward<T>(value)); }

  // Values cannot be moved or copied
  ValueStorage(const ValueStorage& other) = delete;
  ValueStorage(ValueStorage&& other) = delete;
  ValueStorage& operator=(const ValueStorage& other) = delete;
  ValueStorage& operator=(ValueStorage&& other) = delete;

  // The destructor will call the destructor and clean up any allocated storage
  ~ValueStorage() { reset(); }

  // Reset the storage to a new value. This will allocate storage if necessary and set the cleanup function automatically.
  template <typename T> void reset(T&& value);
  // Call this version if you know the currently stored value is not dynamically allocated and trivially destructible.
  template <typename T> void reset_trivial(T&& value);

  // Reset the storeate without a new value
  void reset();
  // Call this version if you know the currently stored value is not dynamically allocated and trivially destructible.
  void reset_trivial();


  // Sets up dynamically allocated storage
  void Allocate(std::size_t alignment, std::size_t size);

  // Deallocates allocated storage. Only call this if Allocate() was called before and the value has already been constructed.
  void Deallocate();

  // Returns true if the storage was dynamically allocated.
  bool has_allocated_storage() const { return flags_.allocated_storage != 0; }

  // Returns the pointer to the allocated storage.
  const void* allocated_storage_pointer() const {
    assert(has_allocated_storage());
    return *reinterpret_cast<void* const*>(&data_);
  }

  // Returns the pointer to the allocated storage.
  void* allocated_storage_pointer() {
    assert(has_allocated_storage());
    return *reinterpret_cast<void* const*>(&data_);
  }

  // Sets the desctuction function for the value.
  void SetDestructFunction(FunctionHandle destructor);
  // Returns the desctruction function.
  FunctionHandle destruct_function() const;

  // Returns a pointer to the internal data. Storage was allocated via Allocate() it will contain the pointer to the storage.
  const void* data() const { return &data_; }
  void* data() { return &data_; }

  const void* value_pointer() const { return has_allocated_storage() ? allocated_storage_pointer() : data(); }
  void* value_pointer() { return has_allocated_storage() ? allocated_storage_pointer() : data(); }

  // Returns the value as a reference to T. THIS METHOD WILL NOT CHECK FOR THE ALLOCATED STORAGE BIT! It assumes that
  // the value is stored internally if possible based on size and alignment.
  template <typename T> T& as();
  template <typename T> const T& as() const;

  // Trivially copies the value from source to destination. Neither the source nor the destination should have allocated storage.
  static void CopyTrivially(ValueStorage* destination, const ValueStorage* source);

 private:
  std::aligned_storage_t<SIZE, ALIGNMENT> data_;
  union {
    std::uintptr_t destruct_function_and_flags;
    // TODO: the following only works on little endian!
    struct {
      std::uintptr_t allocated_storage : 1;
      std::uintptr_t : (sizeof(std::uintptr_t) * 8 - 1);
    } flags_;
  };

  void SetAllocatedStorageFlag(bool value) {
    flags_.allocated_storage = value;
  }

#ifndef NDEBUG
 public:
  TypeId native_type_id_ = TypeOf<void>;
#endif
};

#ifdef NDEBUG
static_assert(sizeof(ValueStorage) == 16);
#endif

// All methods are implemented in the header to help the compiler inline them:

namespace detail {

template <typename T>
void Destruct(void* object) {
  reinterpret_cast<T*>(object)->~T();
}

}  // namespace detail


template <typename T>
inline void ValueStorage::reset(T&& value) {
  reset();

  using StoredType = std::remove_reference_t<T>;
  if constexpr (alignof(T) > ALIGNMENT || sizeof(T) > SIZE) {
    Allocate(alignof(T), sizeof(T));
    SetAllocatedStorageFlag(true);
    new (allocated_storage_pointer()) StoredType(std::forward<T>(value));
  } else {
    new (data()) StoredType(std::forward<T>(value));
  }
  if constexpr (!std::is_trivially_destructible_v<StoredType>) {
    SetDestructFunction(FunctionHandle::FromNativeFunction(&detail::Destruct<StoredType>));
  }
#ifndef NDEBUG
  native_type_id_ = TypeOf<StoredType>;
#endif
}

inline void ValueStorage::reset() {
  const bool is_storage_allocated = has_allocated_storage();
  auto destruct = destruct_function();
  if (destruct) {
    if (destruct.Call<void>(value_pointer())) {
      SetDestructFunction(FunctionHandle::Null());
    } else {
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

inline void ValueStorage::reset_trivial() {
  assert(!has_allocated_storage());
  assert(destruct_function());
#ifndef NDEBUG
  native_type_id_ = TypeOf<void>;
#endif
}

inline void ValueStorage::Allocate(std::size_t alignment, std::size_t size) {
  new (&data_) void*(aligned_alloc(alignment, size));
  SetAllocatedStorageFlag(true);
}

inline void ValueStorage::Deallocate() {
  assert(has_allocated_storage());
  std::free(allocated_storage_pointer());
  SetAllocatedStorageFlag(false);
}

inline void ValueStorage::SetDestructFunction(FunctionHandle destructor) {
  assert((destructor.integer & 1) == 0);
  destruct_function_and_flags = destructor.integer | flags_.allocated_storage;
}

inline FunctionHandle ValueStorage::destruct_function() const {
  return { .integer = destruct_function_and_flags & ~1 };
}

template <typename T>
inline T& ValueStorage::as() {
  // assert(TypeOf<T> == native_type_id_ ||
  //        (!allocated_storage() && destruct_function() == nullptr && std::is_trivially_constructible_v<T>));
#ifndef NDEBUG
  native_type_id_ = TypeOf<T>;
#endif
  if constexpr (stored_inline<T>) {
    return *reinterpret_cast<T*>(data());
  } else {
    return *reinterpret_cast<T*>(allocated_storage_pointer());
  }
}

template <typename T>
inline const T& ValueStorage::as() const {
  assert(TypeOf<T> == native_type_id_);
  if constexpr (stored_inline<T>) {
    return *reinterpret_cast<T*>(data());
  } else {
    return *reinterpret_cast<T*>(allocated_storage_pointer());
  }
}

inline void ValueStorage::CopyTrivially(ValueStorage* destination, const ValueStorage* source) {
  assert(!destination->has_allocated_storage());
  assert(!source->has_allocated_storage());
  std::memcpy(&destination->data_, &source->data_, SIZE);
  destination->destruct_function_and_flags = source->destruct_function_and_flags;
#ifndef NDEBUG
  destination->native_type_id_ = source->native_type_id_;
#endif
}

}  // namespace ovis
