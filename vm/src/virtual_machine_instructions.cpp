#include <ovis/vm/virtual_machine_instructions.hpp>

namespace ovis {
Instruction Instruction::CreateHalt() {
  return { .opcode = OpCode::HALT };
}

Instruction Instruction::CreatePush(std::uint32_t count) {
  return {.push_pop = {.opcode = OpCode::PUSH, .count = count}};
}

Instruction Instruction::CreatePop(std::uint32_t count) {
  return {.push_pop = {.opcode = OpCode::POP, .count = count}};
}

Instruction Instruction::CreatePopTrivial(std::uint32_t count) {
  return {.push_pop = {.opcode = OpCode::POP_TRIVIAL, .count = count}};
}

// Instruction Instruction::CreateCopyTrivialValue(std::uint32_t destination, std::uint32_t source) {
//   return {.copy_trivial_value = {.opcode = OpCode::COPY_TRIVIAL_VALUE,
//                                  .destination = destination,
//                                  .source = source}};
// }

// Instruction Instruction::CreatePushTrivialConstant(std::uint32_t constant_index) {
//   return {
//     .push_trivial_constant = {
//       .opcode = OpCode::PUSH_TRIVIAL_CONSTANT,
//       .constant = constant_index
//     }
//   };
// }

Instruction Instruction::CreateOffsetAddress(std::uint32_t register_index, std::uint32_t offset) {
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

Instruction Instruction::CreateCallNativeFunction(std::uint32_t input_count) {
  return {
    .call_native_function = {
      .opcode = OpCode::CALL_NATIVE_FUNCTION,
      .input_count = input_count
    }
  };
}

Instruction Instruction::CreateJump(std::int32_t offset) {
  return {.jump_data = {.opcode = OpCode::JUMP, .offset = offset}};
}

Instruction Instruction::CreateJumpIfTrue(std::int32_t offset) {
  return {.jump_data = {.opcode = OpCode::JUMP_IF_TRUE, .offset = offset}};
}

Instruction Instruction::CreateJumpIfFalse(std::int32_t offset) {
  return {.jump_data = {.opcode = OpCode::JUMP_IF_FALSE, .offset = offset}};
}

Instruction Instruction::CreateSubtractNumbers(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = OpCode::SUBTRACT_NUMBERS,
      .result = result,
      .first = first,
      .second = second
    }
  };
}

Instruction Instruction::CreateMultiplyNumbers(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = OpCode::MULTIPLY_NUMBERS,
      .result = result,
      .first = first,
      .second = second
    }
  };
}

Instruction Instruction::CreateIsNumberGreater(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
  return {
    .number_operation_data = {
      .opcode = OpCode::IS_NUMBER_GREATER,
      .result = result,
      .first = first,
      .second = second
    }
  };
}

Instruction Instruction::CreateConstructInlineValue() { return {.opcode = OpCode::CONSTRUCT_INLINE_VALUE}; }
Instruction Instruction::CreateConstructValue(std::uint32_t alignment, std::uint32_t size) {
  assert(alignment < (1 << instructions::TYPE_ALIGN_BITS));
  assert(size < (1 << instructions::TYPE_SIZE_BITS));

  return { .construct_value = {
    .opcode = OpCode::CONSTRUCT_VALUE,
    .alignment = alignment,
    .size = size,
  }};
}

Instruction Instruction::CreatePushExecutionState() {
  return {
    .opcode = OpCode::PUSH_EXECUTION_STATE,
  };
}

Instruction Instruction::CreateSetConstantBaseOffset(std::uint32_t base_offset) {
  assert(base_offset < (1 << 24));
  return {
    .set_constant_base_offset_data = {
      .opcode = OpCode::SET_CONSTANT_BASE_OFFSET,
      .base_offset = base_offset,
    }
  };
}

Instruction Instruction::CreateReturn(std::uint32_t output_count) {
  assert(output_count < (1 << 8));
  return {
    .return_data = {
      .opcode = OpCode::RETURN,
      .output_count = output_count,
    },
  };
}
}  // namespace ovis
