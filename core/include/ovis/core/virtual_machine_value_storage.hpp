#pragma once

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <utility>

#include <ovis/utils/type_id.hpp>

namespace ovis {
namespace vm {

class alignas(16) ValueStorage final {
 public:
  using DestructFunction = void(void*);

  constexpr static std::size_t ALIGNMENT = 8;
  constexpr static std::size_t SIZE = 8;

  // We will fit pointers and doubles in the value field, so they should fit and properly aligned
  static_assert(alignof(void*) <= ALIGNMENT);
  static_assert(sizeof(void*) <= SIZE);
  static_assert(alignof(double) <= ALIGNMENT);
  static_assert(sizeof(double) <= SIZE);

  // Constructs an empty value storage
  ValueStorage() : destruct_function_and_flags(0) {}

  // Stores a value of type T in the storage. See reset(T&&)
  template <typename T> ValueStorage(T&& value) : ValueStorage() { reset(std::forward<T>(value)); }

  ValueStorage(const ValueStorage& other) = delete;
  ValueStorage(ValueStorage&& other) = delete;

  // The destructor will clean up any allocated storage and will call the cleanup function.
  ~ValueStorage() { reset(); }

  ValueStorage& operator=(const ValueStorage& other) = delete;
  ValueStorage& operator=(ValueStorage&& other) = delete;

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
  // Deallocates allocated storage. Only call this if Allocate() was called before and the value has already ben destructed.
  void Deallocate();
  // Returns true if the storage was dynamically allocated.
  bool allocated_storage() const { return flags_.allocated_storage != 0; }

  // Sets the desctuction function for the value.
  void SetDestructFunction(DestructFunction* destructor);
  // Returns the desctruction function.
  DestructFunction* destruct_function() const;

  // Returns a pointer to the internal data. Storage was allocated via Allocate() it will contains the pointer to the storage.
  const void* data() const { return &data_; }
  void* data() { return &data_; }

  // Returns a pointer to the allocated storage. Only call this function if storage has been allocated via Allocate().
  const void* data_as_pointer() const {
    assert(allocated_storage());
    return *reinterpret_cast<void* const*>(&data_);
  }
  void* data_as_pointer() {
    assert(allocated_storage());
    return *reinterpret_cast<void* const*>(&data_);
  }

  // Returns the value as a reference to T. THIS METHOD WILL NOT CHECK FOR THE ALLOCATED STORAGE BIT! It assumes that
  // the value is stored internally if possible based on size and alignment.
  template <typename T> T& as();
  template <typename T> const T& as() const;

  // Trivially copies the value from source to destination. Neither the source nor the destination should have allocated storage.
  static void copy_trivially(ValueStorage* destination, const ValueStorage* source);

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
    // See https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
    destruct_function_and_flags ^= (-static_cast<std::uintptr_t>(value) ^ destruct_function_and_flags) & 1;
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
    new (data_as_pointer()) StoredType(std::forward<T>(value));
  } else {
    new (data()) StoredType(std::forward<T>(value));
  }
  if constexpr (!std::is_trivially_destructible_v<T>) {
    SetDestructFunction(&detail::Destruct<StoredType>);
  }
#ifndef NDEBUG
  native_type_id_ = TypeOf<StoredType>;
#endif
}

inline void ValueStorage::reset() {
  const bool is_storage_allocated = allocated_storage();
  DestructFunction* destruct = destruct_function();
  if (destruct) {
    destruct(is_storage_allocated ? data_as_pointer() : data());
    SetDestructFunction(nullptr);
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
  assert(!allocated_storage());
  assert(destruct_function() == nullptr);
#ifndef NDEBUG
  native_type_id_ = TypeOf<void>;
#endif
}

inline void ValueStorage::Allocate(std::size_t alignment, std::size_t size) {
  new (&data_) void*(aligned_alloc(alignment, size));
  SetAllocatedStorageFlag(true);
}

inline void ValueStorage::Deallocate() {
  assert(allocated_storage());
  std::free(*reinterpret_cast<void**>(&data_));
  SetAllocatedStorageFlag(false);
}

inline void ValueStorage::SetDestructFunction(DestructFunction* destructor) {
  const std::uintptr_t pointer_as_int = reinterpret_cast<std::uintptr_t>(destructor);
  assert((pointer_as_int & 1) == 0);
  destruct_function_and_flags = pointer_as_int | flags_.allocated_storage;
}

inline ValueStorage::DestructFunction* ValueStorage::destruct_function() const {
  return reinterpret_cast<DestructFunction*>(destruct_function_and_flags & ~1);
}

template <typename T>
inline T& ValueStorage::as() {
  assert(TypeOf<T> == native_type_id_);
  if constexpr (alignof(T) > ALIGNMENT || sizeof(T) > SIZE) {
    return *reinterpret_cast<T*>(data_as_pointer());
  } else {
    return *reinterpret_cast<T*>(data());
  }
}

template <typename T>
inline const T& ValueStorage::as() const {
  assert(TypeOf<T> == native_type_id_);
  if constexpr (alignof(T) > ALIGNMENT || sizeof(T) > SIZE) {
    return *reinterpret_cast<const T*>(data_as_pointer());
  } else {
    return *reinterpret_cast<const T*>(data());
  }
}

inline void ValueStorage::copy_trivially(ValueStorage* destination, const ValueStorage* source) {
  assert(!destination->allocated_storage());
  assert(!source->allocated_storage());
  std::memcpy(&destination->data_, &source->data_, SIZE);
  destination->destruct_function_and_flags = source->destruct_function_and_flags;
#ifndef NDEBUG
  destination->native_type_id_ = source->native_type_id_;
#endif
}

}  // namespace vm
}  // namespace ovis
