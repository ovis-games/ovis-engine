#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>

namespace ovis {

enum class OpCode : std::uint32_t {
  // Halting script execution
  HALT,

  // Stack manipulation
  PUSH,
  PUSH_TRIVIAL_CONSTANT,
  PUSH_TRIVIAL_STACK_VALUE,
  PUSH_ALLOCATED,
  PUSH_STACK_VALUE_DATA_ADDRESS,
  PUSH_STACK_VALUE_ALLOCATED_ADDRESS,
  PUSH_CONSTANT_DATA_ADDRESS,
  PUSH_CONSTANT_ALLOCATED_ADDRESS,
  POP,
  POP_TRIVIAL,
  ASSIGN_TRIVIAL,

  // Value manipulation
  COPY_TRIVIAL,
  MEMORY_COPY,
  OFFSET_ADDRESS,

  // Function calling
  CALL_NATIVE_FUNCTION,
  PREPARE_SCRIPT_FUNCTION_CALL,
  CALL_SCRIPT_FUNCTION,
  SET_CONSTANT_BASE_OFFSET,
  RETURN,

  // Boolean operations
  NOT,
  AND,
  OR,

  // Number operations
  ADD_NUMBERS,
  SUBTRACT_NUMBERS,
  MULTIPLY_NUMBERS,
  DIVIDE_NUMBERS,
  IS_NUMBER_GREATER,
  IS_NUMBER_LESS,
  IS_NUMBER_GREATER_EQUAL,
  IS_NUMBER_LESS_EQUAL,
  IS_NUMBER_EQUAL,
  IS_NUMBER_NOT_EQUAL,

  // Jumps
  JUMP,
  JUMP_IF_TRUE,
  JUMP_IF_FALSE,

  // OpCode count, not used
  COUNT,
};

namespace instructions {

constexpr std::size_t OPCODE_BITS = 8;
constexpr std::size_t STACK_INDEX_BITS = 12;
constexpr std::size_t TYPE_ALIGN_BITS = 12;
constexpr std::size_t TYPE_SIZE_BITS = 12;
constexpr std::size_t CONSTANT_INDEX_BITS = 12;
constexpr std::size_t ADDRESS_OFFSET_BITS = 12;
constexpr std::size_t JUMP_OFFSET_BITS = 24;
constexpr std::size_t CONSTANT_OFFSET_BITS = 24;

static_assert(static_cast<std::uint32_t>(OpCode::COUNT) < (1 << OPCODE_BITS));

struct StackIndexData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t stack_index : STACK_INDEX_BITS;
};
static_assert(sizeof(StackIndexData) == sizeof(std::uint32_t));

struct ConstantIndexData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t constant_index : CONSTANT_INDEX_BITS;
};
static_assert(sizeof(ConstantIndexData) == sizeof(std::uint32_t));

struct StackAddressesData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t address1 : STACK_INDEX_BITS;
  std::uint32_t address2 : STACK_INDEX_BITS;
};
static_assert(sizeof(StackAddressesData) == sizeof(std::uint32_t));

struct AllocateData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t alignment : TYPE_ALIGN_BITS;
  std::uint32_t size : TYPE_SIZE_BITS;
};
static_assert(sizeof(StackAddressesData) == sizeof(std::uint32_t));

struct CallNativeFunctionData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t input_count : STACK_INDEX_BITS;
};
static_assert(sizeof(CallNativeFunctionData ) == sizeof(std::uint32_t));

struct ReturnData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t output_count : STACK_INDEX_BITS;
};
static_assert(sizeof(ReturnData ) == sizeof(std::uint32_t));

struct JumpData {
  OpCode opcode : OPCODE_BITS;
  std::int32_t offset : JUMP_OFFSET_BITS;
};
static_assert(sizeof(JumpData) == sizeof(std::uint32_t));

struct OffsetAddressData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t stack_index : STACK_INDEX_BITS;
  std::uint32_t offset : ADDRESS_OFFSET_BITS;
};
static_assert(sizeof(OffsetAddressData) == sizeof(std::uint32_t));

struct SetConstantBaseOffsetData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t base_offset : 24;
};
static_assert(sizeof(SetConstantBaseOffsetData) == sizeof(std::uint32_t));

}  // namespace instructions

union Instruction {
  OpCode opcode : instructions::OPCODE_BITS;
  instructions::StackIndexData stack_index_data;
  instructions::ConstantIndexData constant_index_data;
  instructions::AllocateData allocate_data;
  instructions::StackAddressesData stack_addresses_data;
  instructions::CallNativeFunctionData call_native_data;
  instructions::ReturnData return_data;
  instructions::JumpData jump_data;
  instructions::OffsetAddressData offset_address_data;
  instructions::SetConstantBaseOffsetData set_constant_base_offset_data;


  static Instruction CreateHalt();

  static Instruction CreatePush(std::uint32_t count);
  static Instruction CreatePushTrivialConstant(std::uint32_t constant_index);
  static Instruction CreatePushTrivialStackValue(std::uint32_t stack_index);
  static Instruction CreatePushAllocated(std::uint32_t alignment, std::uint32_t size_in_bytes);
  static Instruction CreatePushStackValueDataAddress(std::uint32_t stack_index);
  static Instruction CreatePushStackValueAllocatedAddress(std::uint32_t stack_index);
  static Instruction CreatePushConstantDataAddress(std::uint32_t constant_index);
  static Instruction CreatePushConstantAllocatedAddress(std::uint32_t constant_index);
  static Instruction CreatePop(std::uint32_t count);
  static Instruction CreatePopTrivial(std::uint32_t count);
  static Instruction CreateAssignTrivial(std::uint32_t stack_index);

  static Instruction CreateCopyTrivial(std::uint32_t destination, std::uint32_t source);
  static Instruction CreateMemoryCopy(std::uint32_t size);
  static Instruction CreateOffsetAddress(std::uint32_t stack_index, std::uint32_t offset);

  static Instruction CreateCallNativeFunction(std::uint32_t input_count);
  static Instruction CreatePrepareScriptFunctionCall(std::uint32_t output_count);
  static Instruction CreateScriptFunctionCall(std::uint32_t output_count, std::uint32_t input_count);
  static Instruction CreateSetConstantBaseOffset(std::uint32_t base_offset);
  static Instruction CreateReturn(std::uint32_t output_count);

  static Instruction CreateNot(std::uint32_t input_index);
  static Instruction CreateAnd(std::uint32_t lhs_index, std::uint32_t rhs_index);
  static Instruction CreateOr(std::uint32_t lhs_index, std::uint32_t rhs_index);

  static Instruction CreateAddNumbers();
  static Instruction CreateSubtractNumbers();
  static Instruction CreateMultiplyNumbers();
  static Instruction CreateDivideNumbers();
  static Instruction CreateIsNumberGreater();
  static Instruction CreateIsNumberLess();
  static Instruction CreateIsNumberGreaterEqual();
  static Instruction CreateIsNumberLessEqual();
  static Instruction CreateIsNumberEqual();
  static Instruction CreateIsNumberNotEqual();

  static Instruction CreateJump(std::int32_t offset);
  static Instruction CreateJumpIfTrue(std::int32_t offset);
  static Instruction CreateJumpIfFalse(std::int32_t offset);
};
static_assert(std::is_trivial_v<Instruction>);
static_assert(sizeof(Instruction) == 4);

}  // namespace ovis
