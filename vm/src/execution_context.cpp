#include <ovis/utils/range.hpp>
#include <ovis/vm/execution_context.hpp>
#include <ovis/vm/virtual_machine.hpp>

namespace ovis {

ExecutionContext::ExecutionContext(NotNull<VirtualMachine*> virtual_machine, std::size_t register_count)
    : virtual_machine_(virtual_machine), registers_(std::make_unique<ValueStorage[]>(register_count)) {
  register_count_ = register_count;
  used_register_count_ = 0;
}

ExecutionContext::~ExecutionContext() {
  PopAll();
}

ValueStorage& ExecutionContext::top(std::size_t offset) {
  assert(offset < used_register_count_);
  return registers_[used_register_count_ - (offset + 1)];
}

void ExecutionContext::PushUninitializedValues(std::size_t count) {
  // if (used_register_count_ + count > register_count_) {
  //   return Error("Stack overflow");
  // }
  // for (auto i : IRange(count)) {
  //   assert(registers_[used_register_count_ + i].native_type_id_ == TypeOf<void>);
  // }
  used_register_count_ += count;
  // return Success;
}

void ExecutionContext::PopTrivialValues(std::size_t count) {
  assert(count <= used_register_count_);
  for (auto i : IRange(count)) {
    registers_[used_register_count_ - (i + 1)].ResetTrivial();
  }
  used_register_count_ -= count;
}

void ExecutionContext::PopValues(std::size_t count) {
  assert(count <= used_register_count_);
  for (auto i : IRange(count)) {
    registers_[used_register_count_ - (i + 1)].Reset(this);
  }
  used_register_count_ -= count;
}

std::span<const ValueStorage> ExecutionContext::registers() const {
  return {registers_.get(), used_register_count_};
}

std::span<const ValueStorage> ExecutionContext::current_function_scope_registers() const {
  return {registers_.get(), used_register_count_};
}

Result<> ExecutionContext::Execute(std::uintptr_t instruction_offset) {
  const Instruction* const instructions = virtual_machine()->GetInstructionPointer(0);
  const ValueStorage* const constants = virtual_machine()->GetConstantPointer(0);
  std::size_t program_counter = instruction_offset;

  while (true) {
    Instruction instruction = instructions[program_counter];
    switch (static_cast<OpCode>(instruction.opcode)) {
      case OpCode::HALT: {
        return Success;
      }

      case OpCode::PUSH: {
        PushUninitializedValues(instruction.stack_index_data.stack_index);
        ++program_counter;
        break;
      }

      case OpCode::PUSH_TRIVIAL_CONSTANT: {
        PushUninitializedValues(1);
        ValueStorage::CopyTrivially(
          &top(),
          &constants[constant_offset_ + instruction.constant_index_data.constant_index]
        );
        ++program_counter;
        break;
      }

      case OpCode::PUSH_ALLOCATED: {
        PushUninitializedValues(1);
        top().Allocate(instruction.allocate_data.alignment, instruction.allocate_data.size);
        ++program_counter;
        break;
      }

      case OpCode::PUSH_STACK_VALUE_DATA_ADDRESS: {
        const auto& value = GetStackValue(instruction.stack_index_data.stack_index);
        PushValue(value.data());
        ++program_counter;
        break;
      }

      case OpCode::PUSH_STACK_VALUE_ALLOCATED_ADDRESS: {
        const auto& value = GetStackValue(instruction.stack_index_data.stack_index);
        PushValue(value.allocated_storage_pointer());
        ++program_counter;
        break;
      }

      case OpCode::PUSH_CONSTANT_DATA_ADDRESS: {
        const auto& constant = constants[constant_offset_ + instruction.constant_index_data.constant_index];
        PushValue(constant.data());
        ++program_counter;
        break;
      }

      case OpCode::PUSH_CONSTANT_ALLOCATED_ADDRESS: {
        const auto& constant = constants[constant_offset_ + instruction.constant_index_data.constant_index];
        PushValue(constant.allocated_storage_pointer());
        ++program_counter;
        break;
      }

      case OpCode::POP: {
        PopValues(instruction.stack_index_data.stack_index);
        ++program_counter;
        break;
      }

      case OpCode::POP_TRIVIAL: {
        PopTrivialValues(instruction.stack_index_data.stack_index);
        ++program_counter;
        break;
      }

      case OpCode::ASSIGN_TRIVIAL: {
        ValueStorage::CopyTrivially(&GetStackValue(stack_offset_ + instruction.stack_index_data.stack_index), &top());
        PopTrivialValue();
        ++program_counter;
        break;
      }

      case OpCode::COPY_TRIVIAL: {
        ValueStorage::CopyTrivially(&GetStackValue(stack_offset_ + instruction.stack_index_data.stack_index), &top());
        ++program_counter;
        break;
      }

      case OpCode::MEMORY_COPY: {
        std::memcpy(top(1).as<void*>(), top(0).as<const void*>(), instruction.allocate_data.size);
        PopTrivialValues(2);
        ++program_counter;
        break;
      }

      case OpCode::OFFSET_ADDRESS: {
        GetStackValue<std::uint8_t*>(instruction.offset_address_data.stack_index) +=
            instruction.offset_address_data.offset;
        ++program_counter;
        break;
      }

      case OpCode::CALL_NATIVE_FUNCTION: {
        const auto function_pointer = top().as<NativeFunction*>();
        PopTrivialValue();
        function_pointer(this);
        ++program_counter;
        break;
      }

      case OpCode::PREPARE_SCRIPT_FUNCTION_CALL: {
        assert(false && "Not implemented yet");
      }

      case OpCode::CALL_SCRIPT_FUNCTION: {
        assert(false && "Not implemented yet");
      }

      case OpCode::SET_CONSTANT_BASE_OFFSET: {
        constant_offset_ = instruction.set_constant_base_offset_data.base_offset;
        ++program_counter;
        break;
      }

      case OpCode::RETURN: {
        const auto output_count = instruction.return_data.output_count;
        program_counter = GetStackValue<std::uint32_t>(stack_offset_ + GetReturnAddressOffset(output_count));
        constant_offset_ = GetStackValue<std::uint32_t>(stack_offset_ + GetConstantOffset(output_count));
        const auto old_stack_offset = stack_offset_;
        stack_offset_ = GetStackValue<std::uint32_t>(stack_offset_ + GetConstantOffset(output_count));
        PopValues(used_register_count_ - (old_stack_offset + output_count));
        break;
      }

      case OpCode::NOT: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::AND: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::OR: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::ADD_NUMBERS: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::SUBTRACT_NUMBERS: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::MULTIPLY_NUMBERS: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::DIVIDE_NUMBERS: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::IS_NUMBER_GREATER: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::IS_NUMBER_LESS: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::IS_NUMBER_GREATER_EQUAL: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::IS_NUMBER_LESS_EQUAL: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::IS_NUMBER_EQUAL: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::IS_NUMBER_NOT_EQUAL: {
        assert(false && "Not implemented yet");
        ++program_counter;
        break;
      }

      case OpCode::JUMP: {
        program_counter += instruction.jump_data.offset;
        break;
      }

      case OpCode::JUMP_IF_TRUE: {
        const auto condition = top().as<bool>();
        PopTrivialValue();
        program_counter += condition ? instruction.jump_data.offset : 1;
        break;
      }

      case OpCode::JUMP_IF_FALSE: {
        const auto condition = top().as<bool>();
        PopTrivialValue();
        program_counter += condition ? 1 : instruction.jump_data.offset;
        break;
      }

      default:
        return Error("Invalid instruction opcode: {}", static_cast<uint32_t>(instruction.opcode));
    }
  }

  return Success;
}

}  // namespace ovis
