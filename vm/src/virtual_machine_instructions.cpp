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

Instruction Instruction::CreateMultiplyNumbers(std::uint32_t lhs_index, std::uint32_t rhs_index) {
  return {
    .stack_addresses_data = {
      .opcode = OpCode::MULTIPLY_NUMBERS,
      .address1 = lhs_index,
      .address2 = rhs_index,
    }
  };
}

// Instruction Instruction::CreateOffsetAddress(std::uint32_t register_index, std::uint32_t offset) {
//   assert(register_index < (1 << instructions::REGISTER_INDEX_BITS));
//   assert(offset < (1 << instructions::ADDRESS_OFFSET_BITS));

//   return {
//     .offset_address_data = {
//       .opcode = OpCode::OFFSET_ADDRESS,
//       .register_index = register_index,
//       .offset = offset,
//     }
//   };
// }

// Instruction Instruction::CreateJump(std::int32_t offset) {
//   return {.jump_data = {.opcode = OpCode::JUMP, .offset = offset}};
// }

// Instruction Instruction::CreateJumpIfTrue(std::int32_t offset) {
//   return {.jump_data = {.opcode = OpCode::JUMP_IF_TRUE, .offset = offset}};
// }

// Instruction Instruction::CreateJumpIfFalse(std::int32_t offset) {
//   return {.jump_data = {.opcode = OpCode::JUMP_IF_FALSE, .offset = offset}};
// }

// Instruction Instruction::CreateSubtractNumbers(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
//   return {
//     .number_operation_data = {
//       .opcode = OpCode::SUBTRACT_NUMBERS,
//       .result = result,
//       .first = first,
//       .second = second
//     }
//   };
// }


// Instruction Instruction::CreateIsNumberGreater(std::uint32_t result, std::uint32_t first, std::uint32_t second) {
//   return {
//     .number_operation_data = {
//       .opcode = OpCode::IS_NUMBER_GREATER,
//       .result = result,
//       .first = first,
//       .second = second
//     }
//   };
// }

// Instruction Instruction::CreateConstructInlineValue() { return {.opcode = OpCode::CONSTRUCT_INLINE_VALUE}; }
// Instruction Instruction::CreateConstructValue(std::uint32_t alignment, std::uint32_t size) {
//   assert(alignment < (1 << instructions::TYPE_ALIGN_BITS));
//   assert(size < (1 << instructions::TYPE_SIZE_BITS));

//   return { .construct_value = {
//     .opcode = OpCode::CONSTRUCT_VALUE,
//     .alignment = alignment,
//     .size = size,
//   }};
// }

// Instruction Instruction::CreatePushExecutionState() {
//   return {
//     .opcode = OpCode::PUSH_EXECUTION_STATE,
//   };
// }

}  // namespace ovis
