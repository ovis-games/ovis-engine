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
  PUSH_STACK_VALUE_DATA_ADDRESS,
  PUSH_STACK_VALUE_ALLOCATED_ADDRESS,
  PUSH_CONSTANT_DATA_ADDRESS,
  PUSH_CONSTANT_ALLOCATED_ADDRESS,
  POP,
  POP_TRIVIAL,

  // Value manipulation
  COPY_TRIVIAL,
  ALLOCATE,
  OFFSET_ADDRESS,

  // Function calling
  CALL_NATIVE_FUNCTION,
  PUSH_EXECUTION_STATE,
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

  // Jumps
  JUMP,
  JUMP_IF_TRUE,
  JUMP_IF_FALSE,

  // OpCode count, not used
  COUNT,
};

namespace instructions {

constexpr std::size_t OPCODE_BITS = 8;
constexpr std::size_t STACK_OFFSET_BITS = 8;
constexpr std::size_t TYPE_ALIGN_BITS = 3;
constexpr std::size_t TYPE_SIZE_BITS = 11;
constexpr std::size_t CONSTANT_INDEX_BITS = 10;
constexpr std::size_t JUMP_OFFSET_BITS = 24;
constexpr std::size_t ADDRESS_OFFSET_BITS = 12;

static_assert(static_cast<std::uint32_t>(OpCode::COUNT) < (1 << OPCODE_BITS));

struct StackOffsetData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t offset : STACK_OFFSET_BITS;
};
static_assert(sizeof(StackOffsetData) == sizeof(std::uint32_t));

struct ConstantData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t constant : CONSTANT_INDEX_BITS;
};
static_assert(sizeof(ConstantData) == sizeof(std::uint32_t));

struct StackSourcesDestinationData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t destination : STACK_OFFSET_BITS;
  std::uint32_t source1 : STACK_OFFSET_BITS;
  std::uint32_t source2 : STACK_OFFSET_BITS;
};
static_assert(sizeof(StackSourcesDestinationData) == sizeof(std::uint32_t));

struct CallNativeFunctionData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t input_count : 8;
};
static_assert(sizeof(CallNativeFunctionData ) == sizeof(std::uint32_t));

struct ReturnData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t output_count : 8;
};
static_assert(sizeof(ReturnData ) == sizeof(std::uint32_t));

struct JumpData {
  OpCode opcode : OPCODE_BITS;
  std::int32_t offset : JUMP_OFFSET_BITS;
};
static_assert(sizeof(JumpData) == sizeof(std::uint32_t));

struct NumberOperationData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t result : 8;
  std::uint32_t first : 8;
  std::uint32_t second : 8;
};
static_assert(sizeof(NumberOperationData ) == sizeof(std::uint32_t));

struct OffsetAddressData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t register_index : REGISTER_INDEX_BITS;
  std::uint32_t offset : ADDRESS_OFFSET_BITS;
};
static_assert(sizeof(OffsetAddressData) == sizeof(std::uint32_t));

struct SetConstantBaseOffsetData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t base_offset : 24;
};

}  // namespace instructions

union Instruction {
  OpCode opcode : instructions::OPCODE_BITS;

  instructions::PushPopData push_pop;
  // instructions::ConstructValue construct_value;
  instructions::CopyTrivialValueData copy_trivial_value;
  instructions::PushTrivialConstantData push_trivial_constant;
  instructions::CallNativeFunctionData call_native_function;
  instructions::JumpData jump_data;
  instructions::NumberOperationData number_operation_data;
  instructions::OffsetAddressData offset_address_data;
  instructions::ReturnData return_data;
  instructions::SetConstantBaseOffsetData set_constant_base_offset_data;

  static Instruction CreateHalt();

  static Instruction CreatePush(std::uint32_t count);
  static Instruction CreatePop(std::uint32_t count);
  static Instruction CreatePopTrivial(std::uint32_t count);

  static Instruction CreateOffsetAddress(std::uint32_t register_index, std::uint32_t offset);

  static Instruction CreateCallNativeFunction(std::uint32_t input_count);
  static Instruction CreatePushExecutionState();
  static Instruction CreateSetConstantBaseOffset(std::uint32_t base_offset);
  static Instruction CreateReturn(std::uint32_t output_count);

  static Instruction CreateJump(std::int32_t offset);
  static Instruction CreateJumpIfTrue(std::int32_t offset);
  static Instruction CreateJumpIfFalse(std::int32_t offset);

  static Instruction CreateSubtractNumbers(std::uint32_t result, std::uint32_t first, std::uint32_t second);
  static Instruction CreateMultiplyNumbers(std::uint32_t result, std::uint32_t first, std::uint32_t second);
  static Instruction CreateIsNumberGreater(std::uint32_t result, std::uint32_t first, std::uint32_t second);
};
static_assert(std::is_trivial_v<Instruction>);
static_assert(sizeof(Instruction) == 4);

}  // namespace ovis
