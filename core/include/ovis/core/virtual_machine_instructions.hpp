#pragma once

#include <cstdint>
#include <type_traits>

namespace ovis {
namespace vm {

enum class OpCode : std::uint32_t {
  EXIT,
  PUSH,
  POP,
  POP_TRIVIAL,
  COPY_TRIVIAL_VALUE,
  PUSH_TRIVIAL_CONSTANT,
  CALL_NATIVE_FUNCTION,
  SUBTRACT_NUMBERS,
  MULTIPLY_NUMBERS,
  IS_NUMBER_GREATER,
  JUMP,
  JUMP_IF_TRUE,
  JUMP_IF_FALSE,
};

namespace instructions {

constexpr std::size_t OPCODE_BITS = 8;
constexpr std::size_t TYPE_ALIGN_BITS = 3;
constexpr std::size_t TYPE_SIZE_BITS = 11;
constexpr std::size_t REGISTER_INDEX_BITS = 12;
constexpr std::size_t CONSTANT_INDEX_BITS = 10;
constexpr std::size_t JUMP_OFFSET_BITS = 24;

struct ConstructValue {
  std::uint32_t opcode : OPCODE_BITS;
};
static_assert(sizeof(ConstructValue) == sizeof(std::uint32_t));

struct ConstructLargeValue {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t align : TYPE_ALIGN_BITS;
  std::uint32_t size : TYPE_SIZE_BITS;
};
static_assert(sizeof(ConstructLargeValue) == sizeof(std::uint32_t));

struct DestructValue {
  std::uint32_t opcode : OPCODE_BITS;
};
static_assert(sizeof(DestructValue) == sizeof(std::uint32_t));

struct DestructLargeValue {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t align : TYPE_ALIGN_BITS;
  std::uint32_t size : TYPE_SIZE_BITS;
};
static_assert(sizeof(DestructLargeValue) == sizeof(std::uint32_t));

struct CopyValue {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t destination : REGISTER_INDEX_BITS;
  std::uint32_t source : REGISTER_INDEX_BITS;
};
static_assert(sizeof(CopyValue) == sizeof(std::uint32_t));

struct CopyLargeValue {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t destination : REGISTER_INDEX_BITS;
  std::uint32_t source : REGISTER_INDEX_BITS;
};
static_assert(sizeof(CopyLargeValue) == sizeof(std::uint32_t));

struct CopyTrivialValueData {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t destination : REGISTER_INDEX_BITS;
  std::uint32_t source : REGISTER_INDEX_BITS;
};
static_assert(sizeof(CopyTrivialValueData) == sizeof(std::uint32_t));

struct PushConstant {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t constant : CONSTANT_INDEX_BITS;
};
static_assert(sizeof(PushConstant) == sizeof(std::uint32_t));

struct PushTrivialConstantData {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t constant : CONSTANT_INDEX_BITS;
};
static_assert(sizeof(PushTrivialConstantData) == sizeof(std::uint32_t));

struct PushLargeConstant {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t constant : CONSTANT_INDEX_BITS;
  std::uint32_t align : TYPE_ALIGN_BITS;
  std::uint32_t size : TYPE_SIZE_BITS;
};
static_assert(sizeof(PushLargeConstant) == sizeof(std::uint32_t));

struct PushPopData {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t count : 8;
};
static_assert(sizeof(PushPopData) == sizeof(std::uint32_t));

struct CallNativeFunctionData {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t input_count : 8;
};
static_assert(sizeof(CallNativeFunctionData ) == sizeof(std::uint32_t));

struct JumpData {
  std::uint32_t opcode : OPCODE_BITS;
  std::int32_t offset : JUMP_OFFSET_BITS;
};
static_assert(sizeof(JumpData) == sizeof(std::uint32_t));

struct NumberOperationData {
  std::uint32_t opcode : OPCODE_BITS;
  std::uint32_t result : 8;
  std::uint32_t first : 8;
  std::uint32_t second : 8;
};
static_assert(sizeof(NumberOperationData ) == sizeof(std::uint32_t));

}  // namespace instructions

union Instruction {
  std::uint32_t opcode : instructions::OPCODE_BITS;

  instructions::PushPopData push_pop;
  instructions::PushTrivialConstantData push_trivial_constant;
  instructions::CallNativeFunctionData call_native_function;
  instructions::JumpData jump_data;
  instructions::CopyTrivialValueData copy_trivial_value;
  instructions::NumberOperationData number_operation_data;
};
static_assert(std::is_trivial_v<Instruction>);

namespace instructions {

inline Instruction Exit() {
  return { .opcode = static_cast<std::uint32_t>(OpCode::EXIT) };
}

inline Instruction Push(std::uint32_t count) {
  return {.push_pop = {.opcode = static_cast<std::uint32_t>(OpCode::PUSH), .count = count}};
}

inline Instruction Pop(std::uint32_t count) {
  return {.push_pop = {.opcode = static_cast<std::uint32_t>(OpCode::POP), .count = count}};
}

inline Instruction PopTrivial(std::uint32_t count) {
  return {.push_pop = {.opcode = static_cast<std::uint32_t>(OpCode::POP_TRIVIAL), .count = count}};
}

inline Instruction CopyTrivialValue(std::uint32_t destination, std::uint32_t source) {
  return {.copy_trivial_value = {.opcode = static_cast<std::uint32_t>(OpCode::COPY_TRIVIAL_VALUE),
                                 .destination = destination,
                                 .source = source}};
}

inline Instruction PushTrivialConstant(std::uint32_t constant_index) {
  return {
    .push_trivial_constant = {
      .opcode = static_cast<std::uint32_t>(OpCode::PUSH_TRIVIAL_CONSTANT),
      .constant = constant_index
    }
  };
}

inline Instruction CallNativeFunction(std::uint32_t input_count) {
  return {
    .call_native_function = {
      .opcode = static_cast<std::uint32_t>(OpCode::CALL_NATIVE_FUNCTION),
      .input_count = input_count
    }
  };
}

inline Instruction Jump(std::int32_t offset) {
  return {.jump_data = {.opcode = static_cast<std::uint32_t>(OpCode::JUMP), .offset = offset}};
}

inline Instruction JumpIfTrue(std::int32_t offset) {
  return {.jump_data = {.opcode = static_cast<std::uint32_t>(OpCode::JUMP_IF_TRUE), .offset = offset}};
}

inline Instruction JumpIfFalse(std::int32_t offset) {
  return {.jump_data = {.opcode = static_cast<std::uint32_t>(OpCode::JUMP_IF_FALSE), .offset = offset}};
}

inline Instruction SubtractNumbers(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = static_cast<std::uint32_t>(OpCode::SUBTRACT_NUMBERS),
      .result = result,
      .first = first,
      .second = second
    }
  };
}

inline Instruction MultiplyNumbers(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = static_cast<std::uint32_t>(OpCode::MULTIPLY_NUMBERS),
      .result = result,
      .first = first,
      .second = second
    }
  };
}

inline Instruction IsNumberGreater(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = static_cast<std::uint32_t>(OpCode::IS_NUMBER_GREATER),
      .result = result,
      .first = first,
      .second = second
    }
  };
}

}  // namespace instructions

}  // namespace vm
}  // namespace ovis
