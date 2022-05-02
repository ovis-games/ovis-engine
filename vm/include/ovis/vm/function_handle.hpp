#pragma once

#include <cstdint>
#include <bit>

#include <ovis/utils/result.hpp>

namespace ovis {

class ExecutionContext;
using NativeFunction = Result<>(ExecutionContext*);

namespace detail {
template <typename T> struct IsNativeFunction : std::false_type {};
template <> struct IsNativeFunction<NativeFunction> : std::true_type {};
}
template <typename T> constexpr bool IsNativeFunction = detail::IsNativeFunction<T>::value;

// The handle serves as a "pointer" to a function. You can call functions using the handle, but you need to know the
// amount of types of the parameters. You cannot use the handle to receive the function back (multiple functions may
// have the same handle) this is the biggest difference to the id. The least significant bit is not used and must be
// zero. This can be utilized to store additional information (which is actually done in the value storage!).
union FunctionHandle {
  // We assume the order of the bits below.
  static_assert(std::endian::native == std::endian::little);

  // We assume that a pointer to a function is the same size as other pointers.
  static_assert(sizeof(NativeFunction*) == sizeof(std::uintptr_t));

  operator bool() const {
    return native_function != nullptr;
  }

  // Additionally we assume that all function pointers are aligned to 4 byte boundary. There is no information whether
  // this is the case, however, it seems to hold true in practice.
  NativeFunction* native_function;
  struct {
    // Note: all types here have to be same type!
    std::uintptr_t zero : 1;
    std::uintptr_t is_script_function : 1;
    std::uintptr_t instruction_offset : (sizeof(std::uintptr_t) * 8 - 2);
  };
  std::uintptr_t integer;

  static constexpr FunctionHandle Null() { return FunctionHandle { 0 }; }
  static constexpr FunctionHandle FromNativeFunction(NativeFunction* native_function) {
    return {.native_function = native_function};
  }
  static constexpr FunctionHandle FromScriptFunction(std::uintptr_t instruction_offset) {
    return {
      .zero = 0,
      .is_script_function = 1,
      .instruction_offset = instruction_offset
    };
  }

  static constexpr bool IsValid(const FunctionHandle& handle) {
    assert(handle.zero == 0);
    return handle.native_function != nullptr;
  }
};
static_assert(sizeof(FunctionHandle) == sizeof(std::uintptr_t));
static_assert(std::is_trivially_copyable_v<FunctionHandle>);
static_assert(std::is_trivially_constructible_v<FunctionHandle>);

}  // namespace ovis
