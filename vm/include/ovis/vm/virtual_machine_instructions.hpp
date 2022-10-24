#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>

#include "fmt/core.h"

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

template <>
struct fmt::formatter<ovis::Instruction>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const ovis::Instruction& instruction, FormatContext& ctx) {
    switch (instruction.opcode) {
      case ovis::OpCode::HALT:
        return fmt::format_to(ctx.out(), "{0:+04} | HALT", 0);

      case ovis::OpCode::PUSH:
        return fmt::format_to(ctx.out(), "{0:+04} | PUSH count={0}", (int)instruction.stack_index_data.stack_index);

      case ovis::OpCode::PUSH_TRIVIAL_CONSTANT:
        return fmt::format_to(ctx.out(), "{:+04} | PUSH_TRIVIAL_CONSTANT constant_index={}", 1, instruction.constant_index_data.constant_index);

      case ovis::OpCode::PUSH_TRIVIAL_STACK_VALUE:
        return fmt::format_to(ctx.out(), "{:+04} | PUSH_TRIVIAL_STACK_VALUE stack_index={}", 1, instruction.constant_index_data.constant_index);

      case ovis::OpCode::PUSH_ALLOCATED:
        return fmt::format_to(ctx.out(), "{:+04} | PUSH_ALLOCATED alignment={} size={}", 1, instruction.allocate_data.alignment, instruction.allocate_data.size);

      case ovis::OpCode::PUSH_STACK_VALUE_DATA_ADDRESS:
        return fmt::format_to(ctx.out(), "{:+04} | PUSH_STACK_VALUE_DATA_ADDRESS stack_index={}", 1, instruction.constant_index_data.constant_index);

      case ovis::OpCode::PUSH_STACK_VALUE_ALLOCATED_ADDRESS:
        return fmt::format_to(ctx.out(), "{:+04} | PUSH_STACK_VALUE_ALLOCATED_ADDRESS stack_index={}", 1, instruction.constant_index_data.constant_index);

      case ovis::OpCode::PUSH_CONSTANT_DATA_ADDRESS:
        return fmt::format_to(ctx.out(), "{:+04} | PUSH_CONSTANT_DATA_ADDRESS constant_index={}", 1, instruction.constant_index_data.constant_index);

      case ovis::OpCode::PUSH_CONSTANT_ALLOCATED_ADDRESS:
        return fmt::format_to(ctx.out(), "{:+04} | PUSH_CONSTANT_ALLOCATED_ADDRESS constant_index={}", 1, instruction.constant_index_data.constant_index);

      case ovis::OpCode::POP:
        return fmt::format_to(ctx.out(), "{:+04} | POP count={}", -instruction.stack_index_data.stack_index, instruction.stack_index_data.stack_index);

      case ovis::OpCode::POP_TRIVIAL:
        return fmt::format_to(ctx.out(), "{:+04} | POP_TRIVIAL count={}", -instruction.stack_index_data.stack_index, instruction.stack_index_data.stack_index);

      case ovis::OpCode::ASSIGN_TRIVIAL:
        return fmt::format_to(ctx.out(), "{:+04} | ASSIGN_TRIVIAL destination={}", -1, instruction.stack_index_data.stack_index);

      case ovis::OpCode::COPY_TRIVIAL:
        return fmt::format_to(ctx.out(), "{:+04} | COPY_TRIVIAL destination={} source={}", 0, instruction.stack_addresses_data.address1, instruction.stack_addresses_data.address2);

      case ovis::OpCode::MEMORY_COPY:
        return fmt::format_to(ctx.out(), "{:+04} | MEMORY_COPY size={}", -2, instruction.allocate_data.size);

      case ovis::OpCode::OFFSET_ADDRESS:
        return fmt::format_to(ctx.out(), "{:+04} | OFFSET_ADDRESS stack_index={} offset={}", 0, instruction.offset_address_data.stack_index, instruction.offset_address_data.offset);

      case ovis::OpCode::CALL_NATIVE_FUNCTION:
        return fmt::format_to(ctx.out(), "{:+04} | CALL_NATIVE_FUNCTION input_count={}", -1, instruction.call_native_data.input_count);

      case ovis::OpCode::PREPARE_SCRIPT_FUNCTION_CALL:
        return fmt::format_to(ctx.out(), "{:+04} | PREPARE_SCRIPT_FUNCTION_CALL output_count={}", 3, instruction.stack_index_data.stack_index);

      case ovis::OpCode::CALL_SCRIPT_FUNCTION:
        return fmt::format_to(ctx.out(), "{:+04} | CALL_SCRIPT_FUNCTION input_count={} output_count={}", -1, instruction.stack_addresses_data.address1, instruction.stack_addresses_data.address2);

      case ovis::OpCode::SET_CONSTANT_BASE_OFFSET:
        return fmt::format_to(ctx.out(), "{:+04} | SET_CONSTANT_BASE_OFFSET base_offset={}", -1, instruction.set_constant_base_offset_data.base_offset);

      case ovis::OpCode::RETURN:
        return fmt::format_to(ctx.out(), "{:+04} | RETURN output_count={}", 0, instruction.return_data.output_count);

      case ovis::OpCode::NOT:
        return fmt::format_to(ctx.out(), "{:+04} | NOT input_index={}", 1, instruction.stack_addresses_data.address1);

      case ovis::OpCode::AND:
        return fmt::format_to(ctx.out(), "{:+04} | AND lhs_index={} rhs_index={}", 1, instruction.stack_addresses_data.address1, instruction.stack_addresses_data.address2);

      case ovis::OpCode::OR:
        return fmt::format_to(ctx.out(), "{:+04} | OR lhs_index={} rhs_index={}", 1, instruction.stack_addresses_data.address1, instruction.stack_addresses_data.address2);

      case ovis::OpCode::ADD_NUMBERS:
        return fmt::format_to(ctx.out(), "{:+04} | ADD_NUMBERS", -1);

      case ovis::OpCode::SUBTRACT_NUMBERS:
        return fmt::format_to(ctx.out(), "{:+04} | SUBTRACT_NUMBERS", -1);

      case ovis::OpCode::MULTIPLY_NUMBERS:
        return fmt::format_to(ctx.out(), "{:+04} | MULTIPLY_NUMBERS", -1);

      case ovis::OpCode::DIVIDE_NUMBERS:
        return fmt::format_to(ctx.out(), "{:+04} | DIVIDE_NUMBERS", -1);

      case ovis::OpCode::IS_NUMBER_GREATER:
        return fmt::format_to(ctx.out(), "{:+04} | IS_NUMBER_GREATER", -1);

      case ovis::OpCode::IS_NUMBER_LESS:
        return fmt::format_to(ctx.out(), "{:+04} | IS_NUMBER_LESS", -1);

      case ovis::OpCode::IS_NUMBER_GREATER_EQUAL:
        return fmt::format_to(ctx.out(), "{:+04} | IS_NUMBER_GREATER_EQUAL", -1);

      case ovis::OpCode::IS_NUMBER_LESS_EQUAL:
        return fmt::format_to(ctx.out(), "{:+04} | IS_NUMBER_LESS_EQUAL", -1);

      case ovis::OpCode::IS_NUMBER_EQUAL:
        return fmt::format_to(ctx.out(), "{:+04} | IS_NUMBER_EQUAL", -1);

      case ovis::OpCode::IS_NUMBER_NOT_EQUAL:
        return fmt::format_to(ctx.out(), "{:+04} | IS_NUMBER_NOT_EQUAL", -1);

      case ovis::OpCode::JUMP:
        return fmt::format_to(ctx.out(), "{:+04} | JUMP offset={}", 0, instruction.jump_data.offset);

      case ovis::OpCode::JUMP_IF_TRUE:
        return fmt::format_to(ctx.out(), "{:+04} | JUMP_IF_TRUE offset={}", -1, instruction.jump_data.offset);

      case ovis::OpCode::JUMP_IF_FALSE:
        return fmt::format_to(ctx.out(), "{:+04} | JUMP_IF_FALSE offset={}", -1, instruction.jump_data.offset);

      case ovis::OpCode::COUNT:
        return fmt::format_to(ctx.out(), "COUNT");
    }
  }
};
