#include <ovis/vm/virtual_machine_instructions.hpp>

namespace ovis {

Instruction Instruction::CreateHalt() {
  return { .opcode = OpCode::HALT };
}

Instruction Instruction::CreatePush(std::uint32_t count) {
  assert(count < (1 << instructions::STACK_INDEX_BITS));
  return {
    .stack_index_data = {
      .opcode = OpCode::PUSH,
      .stack_index = count
    }
  };
}

Instruction Instruction::CreatePushTrivialConstant(std::uint32_t constant_index) {
  assert(constant_index < (1 << instructions::CONSTANT_INDEX_BITS));
  return {
    .constant_index_data = {
      .opcode = OpCode::PUSH_TRIVIAL_CONSTANT,
      .constant_index = constant_index,
    }
  };
}

Instruction Instruction::CreatePushTrivialStackValue(std::uint32_t stack_index) {
  assert(stack_index < (1 << instructions::STACK_INDEX_BITS));
  return {
    .stack_index_data = {
      .opcode = OpCode::PUSH_TRIVIAL_STACK_VALUE,
      .stack_index = stack_index,
    }
  };
}

Instruction Instruction::CreatePushAllocated(std::uint32_t alignment, std::uint32_t size_in_bytes) {
  assert(alignment < (1 << instructions::TYPE_ALIGN_BITS));
  assert(size_in_bytes < (1 << instructions::TYPE_SIZE_BITS));
  return {
    .allocate_data = {
      .opcode = OpCode::PUSH_ALLOCATED,
      .alignment = alignment,
      .size = size_in_bytes,
    }
  };
}

Instruction Instruction::CreatePushStackValueDataAddress(std::uint32_t stack_index) {
  assert(stack_index < (1 << instructions::STACK_INDEX_BITS));
  return {
    .stack_index_data = {
      .opcode = OpCode::PUSH_STACK_VALUE_DATA_ADDRESS,
      .stack_index = stack_index,
    }
  };
}

Instruction Instruction::CreatePushStackValueAllocatedAddress(std::uint32_t stack_index) {
  assert(stack_index < (1 << instructions::STACK_INDEX_BITS));

  return {
    .stack_index_data = {
      .opcode = OpCode::PUSH_STACK_VALUE_ALLOCATED_ADDRESS,
      .stack_index = stack_index,
    }
  };
}

Instruction Instruction::CreatePushConstantDataAddress(std::uint32_t constant_index) {
  assert(constant_index < (1 << instructions::CONSTANT_INDEX_BITS));

  return {
    .constant_index_data = {
      .opcode = OpCode::PUSH_CONSTANT_DATA_ADDRESS,
      .constant_index = constant_index,
    }
  };
}

Instruction Instruction::CreatePushConstantAllocatedAddress(std::uint32_t constant_index) {
  assert(constant_index < (1 << instructions::CONSTANT_INDEX_BITS));
  return {
    .constant_index_data = {
      .opcode = OpCode::PUSH_CONSTANT_ALLOCATED_ADDRESS,
      .constant_index = constant_index,
    }
  };
}

Instruction Instruction::CreatePop(std::uint32_t count) {
  assert(count < (1 << instructions::STACK_INDEX_BITS));
  return {
    .stack_index_data = {
      .opcode = OpCode::POP,
      .stack_index = count
    }
  };
}

Instruction Instruction::CreateAssignTrivial(std::uint32_t stack_index) {
  assert(stack_index < (1 << instructions::STACK_INDEX_BITS));
  return {
    .stack_index_data = {
      .opcode = OpCode::ASSIGN_TRIVIAL,
      .stack_index = stack_index
    }
  };
}

Instruction Instruction::CreatePopTrivial(std::uint32_t count) {
  assert(count < (1 << instructions::STACK_INDEX_BITS));
  return {
    .stack_index_data = {
      .opcode = OpCode::POP_TRIVIAL,
      .stack_index = count
    }
  };
}

Instruction Instruction::CreateCopyTrivial(std::uint32_t destination, std::uint32_t source) {
  assert(destination < (1 << instructions::STACK_INDEX_BITS));
  assert(source < (1 << instructions::STACK_INDEX_BITS));
  return {
    .stack_addresses_data = {
      .opcode = OpCode::COPY_TRIVIAL,
      .address1 = destination,
      .address2 = source,
    }
  };
}

Instruction Instruction::CreateMemoryCopy(std::uint32_t size) {
  assert(size < (1 << instructions::TYPE_SIZE_BITS));
  return {
    .allocate_data = {
      .opcode = OpCode::MEMORY_COPY,
      .size = size,
    }
  };
}

Instruction Instruction::CreateOffsetAddress(std::uint32_t stack_index, std::uint32_t offset) {
  assert(stack_index < (1 << instructions::STACK_INDEX_BITS));
  assert(offset < (1 << instructions::ADDRESS_OFFSET_BITS));
  return {
    .offset_address_data = {
      .opcode = OpCode::OFFSET_ADDRESS,
      .stack_index = stack_index,
      .offset = offset,
    }
  };
}

Instruction Instruction::CreateCallNativeFunction(std::uint32_t input_count) {
  assert(input_count < (1 << instructions::STACK_INDEX_BITS));
  return {
    .stack_index_data = {
      .opcode = OpCode::CALL_NATIVE_FUNCTION,
      .stack_index = input_count
    }
  };
}

Instruction Instruction::CreatePrepareScriptFunctionCall(std::uint32_t output_count) {
  assert(output_count < (1 << instructions::STACK_INDEX_BITS));

  return {
    .stack_index_data = {
      .opcode = OpCode::PREPARE_SCRIPT_FUNCTION_CALL,
      .stack_index = output_count
    }
  };
}

Instruction Instruction::CreateScriptFunctionCall(std::uint32_t output_count, std::uint32_t input_count) {
  assert(output_count < (1 << instructions::STACK_INDEX_BITS));
  assert(input_count < (1 << instructions::STACK_INDEX_BITS));

  return {
    .stack_addresses_data = {
      .opcode = OpCode::CALL_SCRIPT_FUNCTION,
      .address1 = output_count,
      .address2 = input_count,
    }
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

Instruction Instruction::CreateNot(std::uint32_t input_index) {
  assert(input_index < (1 << instructions::STACK_INDEX_BITS));

  return {
    .stack_addresses_data = {
      .opcode = OpCode::NOT,
      .address1 = input_index,
    }
  };
}

Instruction Instruction::CreateAnd(std::uint32_t lhs_index, std::uint32_t rhs_index) {
  assert(lhs_index < (1 << instructions::STACK_INDEX_BITS));
  assert(rhs_index < (1 << instructions::STACK_INDEX_BITS));

  return {
    .stack_addresses_data = {
      .opcode = OpCode::AND,
      .address1 = lhs_index,
      .address2 = rhs_index,
    }
  };
}

Instruction Instruction::CreateOr(std::uint32_t lhs_index, std::uint32_t rhs_index) {
  assert(lhs_index < (1 << instructions::STACK_INDEX_BITS));
  assert(rhs_index < (1 << instructions::STACK_INDEX_BITS));

  return {
    .stack_addresses_data = {
      .opcode = OpCode::OR,
      .address1 = lhs_index,
      .address2 = rhs_index,
    }
  };
}

Instruction Instruction::CreateAddNumbers() {
  return {
    .opcode = OpCode::ADD_NUMBERS,
  };
}

Instruction Instruction::CreateSubtractNumbers() {
  return {
   .opcode= OpCode::SUBTRACT_NUMBERS,
  };
}

Instruction Instruction::CreateMultiplyNumbers() {
  return {
    .opcode = OpCode::MULTIPLY_NUMBERS,
  };
}

Instruction Instruction::CreateDivideNumbers() {
  return {
    .opcode = OpCode::DIVIDE_NUMBERS,
  };
}

Instruction Instruction::CreateIsNumberGreater() {
  return {
    .opcode = OpCode::IS_NUMBER_GREATER,
  };
}

Instruction Instruction::CreateIsNumberLess() {
  return {
    .opcode = OpCode::IS_NUMBER_LESS,
  };
}

Instruction Instruction::CreateIsNumberGreaterEqual() {
  return {
    .opcode = OpCode::IS_NUMBER_GREATER_EQUAL,
  };
}

Instruction Instruction::CreateIsNumberLessEqual() {
  return {
   .opcode= OpCode::IS_NUMBER_LESS_EQUAL,
  };
}

Instruction Instruction::CreateIsNumberEqual() {
  return {
   .opcode= OpCode::IS_NUMBER_EQUAL,
  };
}

Instruction Instruction::CreateIsNumberNotEqual() {
  return {
    .opcode = OpCode::IS_NUMBER_NOT_EQUAL,
  };
}

Instruction Instruction::CreateJump(std::int32_t offset) {
  assert(offset < (1 << instructions::JUMP_OFFSET_BITS));

  return {
    .jump_data = {
      .opcode = OpCode::JUMP,
      .offset = offset
    }
  };
}

Instruction Instruction::CreateJumpIfTrue(std::int32_t offset) {
  assert(offset < (1 << instructions::JUMP_OFFSET_BITS));

  return {
    .jump_data = {
      .opcode = OpCode::JUMP_IF_TRUE,
      .offset = offset
    }
  };
}

Instruction Instruction::CreateJumpIfFalse(std::int32_t offset) {
  assert(offset < (1 << instructions::JUMP_OFFSET_BITS));

  return {
    .jump_data = {
      .opcode = OpCode::JUMP_IF_FALSE,
      .offset = offset
    }
  };
}

}  // namespace ovis
