#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>

namespace ovis {

enum class OpCode : std::uint32_t {
  EXIT,
  PUSH,
  POP,
  POP_TRIVIAL,
  CONSTRUCT_INLINE_VALUE,
  CONSTRUCT_VALUE,
  COPY_TRIVIAL_VALUE,
  PUSH_TRIVIAL_CONSTANT,

  CALL_NATIVE_FUNCTION,
  CALL_SCRIPT_FUNCTION,
  SET_CONSTANT_BASE_OFFSET,
  RETURN,

  SUBTRACT_NUMBERS,
  MULTIPLY_NUMBERS,
  IS_NUMBER_GREATER,
  JUMP,
  JUMP_IF_TRUE,
  JUMP_IF_FALSE,

  OFFSET_ADDRESS,
};

namespace instructions {

constexpr std::size_t OPCODE_BITS = 8;
constexpr std::size_t TYPE_ALIGN_BITS = 3;
constexpr std::size_t TYPE_SIZE_BITS = 11;
constexpr std::size_t REGISTER_INDEX_BITS = 12;
constexpr std::size_t CONSTANT_INDEX_BITS = 10;
constexpr std::size_t JUMP_OFFSET_BITS = 24;
constexpr std::size_t ADDRESS_OFFSET_BITS = 12;

struct ConstructInlineValue {
  OpCode opcode : OPCODE_BITS;
};
static_assert(sizeof(ConstructInlineValue) == sizeof(std::uint32_t));

struct ConstructValue {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t alignment : TYPE_ALIGN_BITS;
  std::uint32_t size : TYPE_SIZE_BITS;
};
static_assert(sizeof(ConstructValue) == sizeof(std::uint32_t));

struct DestructInlineValue {
  OpCode opcode : OPCODE_BITS;
};
static_assert(sizeof(DestructInlineValue) == sizeof(std::uint32_t));

struct DestructValue {
  OpCode opcode : OPCODE_BITS;
  // std::uint32_t align : TYPE_ALIGN_BITS;
  // std::uint32_t size : TYPE_SIZE_BITS;
};
static_assert(sizeof(DestructValue) == sizeof(std::uint32_t));

struct CopyValue {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t destination : REGISTER_INDEX_BITS;
  std::uint32_t source : REGISTER_INDEX_BITS;
};
static_assert(sizeof(CopyValue) == sizeof(std::uint32_t));

struct CopyLargeValue {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t destination : REGISTER_INDEX_BITS;
  std::uint32_t source : REGISTER_INDEX_BITS;
};
static_assert(sizeof(CopyLargeValue) == sizeof(std::uint32_t));

struct CopyTrivialValueData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t destination : REGISTER_INDEX_BITS;
  std::uint32_t source : REGISTER_INDEX_BITS;
};
static_assert(sizeof(CopyTrivialValueData) == sizeof(std::uint32_t));

struct PushConstant {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t constant : CONSTANT_INDEX_BITS;
};
static_assert(sizeof(PushConstant) == sizeof(std::uint32_t));

struct PushTrivialConstantData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t constant : CONSTANT_INDEX_BITS;
};
static_assert(sizeof(PushTrivialConstantData) == sizeof(std::uint32_t));

struct PushLargeConstant {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t constant : CONSTANT_INDEX_BITS;
  std::uint32_t align : TYPE_ALIGN_BITS;
  std::uint32_t size : TYPE_SIZE_BITS;
};
static_assert(sizeof(PushLargeConstant) == sizeof(std::uint32_t));

struct PushPopData {
  OpCode opcode : OPCODE_BITS;
  std::uint32_t count : 8;
};
static_assert(sizeof(PushPopData) == sizeof(std::uint32_t));

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
  instructions::ConstructValue construct_value;
  instructions::PushTrivialConstantData push_trivial_constant;
  instructions::CallNativeFunctionData call_native_function;
  instructions::JumpData jump_data;
  instructions::CopyTrivialValueData copy_trivial_value;
  instructions::NumberOperationData number_operation_data;
  instructions::OffsetAddressData offset_address_data;
  instructions::ReturnData return_data;
  instructions::SetConstantBaseOffsetData set_constant_base_offset_data;

  static Instruction CreatePushTrivialConstant(std::uint32_t constant_index) {
    assert(constant_index < (1 << instructions::CONSTANT_INDEX_BITS));
    return {
      .push_trivial_constant = {
        .opcode = OpCode::PUSH_TRIVIAL_CONSTANT,
        .constant = constant_index,
      }
    };
  }

  static Instruction CreateConstructInlineValue() { return {.opcode = OpCode::CONSTRUCT_INLINE_VALUE}; }
  static Instruction CreateConstructValue(std::uint32_t alignment, std::uint32_t size) {
    assert(alignment < (1 << instructions::TYPE_ALIGN_BITS));
    assert(size < (1 << instructions::TYPE_SIZE_BITS));

    return { .construct_value = {
      .opcode = OpCode::CONSTRUCT_VALUE,
      .alignment = alignment,
      .size = size,
    }};
  }

  static Instruction CreateReturn(std::uint32_t output_count) {
    assert(output_count < (1 << 8));
    return {
      .return_data = {
        .opcode = OpCode::RETURN,
        .output_count = output_count,
      },
    };
  }

  static Instruction CreateOffsetAddress(std::uint32_t register_index, std::uint32_t offset) {
    assert(register_index < (1 << instructions::REGISTER_INDEX_BITS));
    assert(offset < (1 << instructions::ADDRESS_OFFSET_BITS));

    return {
      .offset_address_data = {
        .opcode = OpCode::OFFSET_ADDRESS,
        .register_index = register_index,
        .offset = offset,
      }
    };
  }

  static Instruction CreateSetConstantBaseOffset(std::uint32_t base_offset) {
    assert(base_offset < (1 << 24));
    return {
      .set_constant_base_offset_data = {
        .opcode = OpCode::SET_CONSTANT_BASE_OFFSET,
        .base_offset = base_offset,
      }
    };
  }
};
static_assert(std::is_trivial_v<Instruction>);
static_assert(sizeof(Instruction) == 4);

namespace instructions {

inline Instruction Exit() {
  return { .opcode = OpCode::EXIT };
}

inline Instruction Push(std::uint32_t count) {
  return {.push_pop = {.opcode = OpCode::PUSH, .count = count}};
}

inline Instruction Pop(std::uint32_t count) {
  return {.push_pop = {.opcode = OpCode::POP, .count = count}};
}

inline Instruction PopTrivial(std::uint32_t count) {
  return {.push_pop = {.opcode = OpCode::POP_TRIVIAL, .count = count}};
}

inline Instruction CopyTrivialValue(std::uint32_t destination, std::uint32_t source) {
  return {.copy_trivial_value = {.opcode = OpCode::COPY_TRIVIAL_VALUE,
                                 .destination = destination,
                                 .source = source}};
}

inline Instruction PushTrivialConstant(std::uint32_t constant_index) {
  return {
    .push_trivial_constant = {
      .opcode = OpCode::PUSH_TRIVIAL_CONSTANT,
      .constant = constant_index
    }
  };
}

inline Instruction CallNativeFunction(std::uint32_t input_count) {
  return {
    .call_native_function = {
      .opcode = OpCode::CALL_NATIVE_FUNCTION,
      .input_count = input_count
    }
  };
}

inline Instruction Jump(std::int32_t offset) {
  return {.jump_data = {.opcode = OpCode::JUMP, .offset = offset}};
}

inline Instruction JumpIfTrue(std::int32_t offset) {
  return {.jump_data = {.opcode = OpCode::JUMP_IF_TRUE, .offset = offset}};
}

inline Instruction JumpIfFalse(std::int32_t offset) {
  return {.jump_data = {.opcode = OpCode::JUMP_IF_FALSE, .offset = offset}};
}

inline Instruction SubtractNumbers(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = OpCode::SUBTRACT_NUMBERS,
      .result = result,
      .first = first,
      .second = second
    }
  };
}

inline Instruction MultiplyNumbers(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = OpCode::MULTIPLY_NUMBERS,
      .result = result,
      .first = first,
      .second = second
    }
  };
}

inline Instruction IsNumberGreater(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = OpCode::IS_NUMBER_GREATER,
      .result = result,
      .first = first,
      .second = second
    }
  };
}

}  // namespace instructions

}  // namespace ovis
