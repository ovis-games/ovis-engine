#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <ovis/utils/native_type_id.hpp>
#include <ovis/utils/not_null.hpp>
#include <ovis/vm/function_handle.hpp>

namespace ovis {

class alignas(16) ValueStorage final {
 public:
  constexpr static std::size_t ALIGNMENT = 8;
  constexpr static std::size_t SIZE = 8;

  static constexpr bool IsTypeStoredInline(std::size_t alignment, std::size_t size) {
    return alignment <= ALIGNMENT && size <= SIZE;
  }

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
  ~ValueStorage() { assert(!destruct_function()); assert(!has_allocated_storage()); }

  // Stores a value inside the ValueStorage
  template <typename T> void Store(T&& value);

  // Destroys the value inside the storage
  void Reset(NotNull<ExecutionContext*> execution_context);

  // Call this version if you know the currently stored value is not dynamically allocated and trivially destructible.
  void ResetTrivial();

  // Allocates storage if the value defined by alignment and size cannot be stored inline.
  void* AllocateIfNecessary(std::size_t alignment, std::size_t size);

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
  NativeTypeId native_type_id_ = TypeOf<void>;
#endif
};

#ifdef NDEBUG
static_assert(sizeof(ValueStorage) == 16);
#endif

}  // namespace ovis

#include <ovis/vm/execution_context.hpp>
#include <ovis/vm/type_helper.hpp>

namespace ovis {

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
    return *reinterpret_cast<const T*>(data());
  } else {
    return *reinterpret_cast<const T*>(allocated_storage_pointer());
  }
}


template <typename T>
inline void ValueStorage::Store(T&& value) {
  assert(!destruct_function());
  assert(!has_allocated_storage());

  using StoredType = std::remove_reference_t<T>;
  if constexpr (alignof(T) > ALIGNMENT || sizeof(T) > SIZE) {
    Allocate(alignof(T), sizeof(T));
    SetAllocatedStorageFlag(true);
    new (allocated_storage_pointer()) StoredType(std::forward<T>(value));
  } else {
    new (data()) StoredType(std::forward<T>(value));
  }
  if constexpr (!std::is_trivially_destructible_v<StoredType>) {
    SetDestructFunction(FunctionHandle::FromNativeFunction(&NativeFunctionWrapper<&type_helper::Destruct<StoredType>>));
  }
#ifndef NDEBUG
  native_type_id_ = TypeOf<StoredType>;
#endif
}

}  // namespace ovis
