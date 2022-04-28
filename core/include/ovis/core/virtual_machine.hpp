#pragma once

// #include <any>
// #include <cassert>
#include <cstddef>
// #include <cstdlib>
#include <memory>
// #include <optional>
#include <span>
// #include <string>
// #include <string_view>
// #include <type_traits>
// #include <typeindex>
// #include <typeinfo>
// #include <vector>

// #include <ovis/utils/down_cast.hpp>
// #include <ovis/utils/json.hpp>
// #include <ovis/utils/native_type_id.hpp>
// #include <ovis/utils/parameter_pack.hpp>
// #include <ovis/utils/range.hpp>
// #include <ovis/utils/reflection.hpp>
// #include <ovis/utils/result.hpp>
// #include <ovis/utils/safe_pointer.hpp>
// #include <ovis/utils/type_list.hpp>
// #include <ovis/utils/versioned_index.hpp>
// #include <ovis/core/function_handle.hpp>
// #include <ovis/core/type_helper.hpp>
// #include <ovis/core/value_storage.hpp>
#include <ovis/core/virtual_machine_instructions.hpp>

namespace ovis {

// Forward declarations
class Type;
class Value;
class Function;
class Module;
class ExecutionContext;
class ValueStorage;

// template <typename T> constexpr bool is_reference_type_v = std::is_base_of_v<SafelyReferenceable, T>;
// template <typename T> constexpr bool is_pointer_to_reference_type_v = std::is_pointer_v<T> && std::is_base_of_v<SafelyReferenceable, std::remove_pointer_t<T>>;
// template <typename T> constexpr bool is_value_type_v = !std::is_base_of_v<SafelyReferenceable, T> && !std::is_pointer_v<T> && !std::is_same_v<std::remove_cvref_t<T>, Value>;
// template <typename T> constexpr bool is_pointer_to_value_type_v = std::is_pointer_v<T> && is_value_type_v<std::remove_pointer_t<T>>;

// template <typename T> concept ReferenceType = is_reference_type_v<T>;
// template <typename T> concept PointerToReferenceType = is_pointer_to_reference_type_v<T>;
// template <typename T> concept ValueType = is_value_type_v<T>;
// template <typename T> concept PointerToValueType = is_pointer_to_value_type_v<T>;


// namespace detail {
// template <typename T> struct TypeWrapper { using type = T; };
// template <typename T> struct TypeWrapper<T&> { using type = std::reference_wrapper<T>; };
// template <typename... T> struct TypeWrapper<std::tuple<T...>> { using type = std::tuple<typename TypeWrapper<T>::type...>; };
// }
// template <typename T> using WrappedType = typename detail::TypeWrapper<T>::type;

class VirtualMachine {
 public:
  VirtualMachine(std::size_t constants_capacity = 1024, std::size_t instruction_capacity = 1024 * 1024);

  // Registers a new function with the specified instructions and constants
  std::size_t RegisterFunction(std::span<const vm::Instruction> instructions, std::span<const Value> constants);
  const vm::Instruction* GetInstructionPointer(std::size_t offset) const;
  std::span<ValueStorage> GetConstantRange(std::size_t offset, std::size_t count);

 private:
  std::unique_ptr<ValueStorage[]> constants_;
  std::unique_ptr<vm::Instruction[]> instructions;
};

namespace vm {

// Allocates count instructions in the vm. The offset from the beginning is returned. A span of the allocated
// instructions can be returned via GetInstructionRange(AllocateInstructions(count), count).
std::size_t AllocateInstructions(std::size_t count);
std::span<Instruction> GetInstructionRange(std::size_t offset, std::size_t count);

std::size_t AllocateConstants(std::size_t count);
std::span<ValueStorage> GetConstantRange(std::size_t offset, std::size_t count);

}  // namespace vm


// Implementation




}  // namespace ovis
