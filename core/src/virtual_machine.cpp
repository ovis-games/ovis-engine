#include <ovis/core/function.hpp>
#include <ovis/core/type.hpp>
#include <ovis/core/virtual_machine.hpp>

namespace ovis {

ExecutionContext ExecutionContext::global;

namespace vm {

namespace {
std::vector<Instruction> code;
std::array<ValueStorage, 1024> data;
std::size_t data_value_count = 0;
}  // namespace

std::size_t AllocateInstructions(std::size_t count) {
  const std::size_t offset = code.size();
  // TODO: Add "UNUSED INSTRUCTION"
  code.resize(code.size() + count);
  return offset;
}

std::span<Instruction> GetInstructionRange(std::size_t offset, std::size_t count) {
  assert(offset + count <= code.size());
  return { code.data() + offset, count };
}

std::size_t AllocateConstants(std::size_t count) {
  assert(data_value_count + count <= data.size());
  std::size_t offset = data_value_count;
  data_value_count += count;
  return offset;
}

std::span<ValueStorage> GetConstantRange(std::size_t offset, std::size_t count) {
  assert(offset + count <= data.size());
  return { data.data() + offset, count };
}

}  // namespace vm

ExecutionContext::ExecutionContext(std::size_t register_count) {
  registers_ = std::make_unique<ValueStorage[]>(register_count);
  register_count_ = register_count;
  used_register_count_ = 0;
  // stack_frames_.push({ .register_offset = 0 });
}

Result<> ExecutionContext::Execute(std::span<const vm::Instruction> instructions, std::span<const ValueStorage> constants) {
  std::size_t program_counter = 0;
  while (program_counter < instructions.size()) {
    vm::Instruction instruction = instructions[program_counter];
    switch (static_cast<vm::OpCode>(instruction.opcode)) {
      case vm::OpCode::EXIT: {
        return Success;
      }

      case vm::OpCode::PUSH: {
        PushValues(instruction.push_pop.count);
        ++program_counter;
        break;
      }

      case vm::OpCode::POP: {
        PopValues(instruction.push_pop.count);
        ++program_counter;
        break;
      }

      case vm::OpCode::POP_TRIVIAL: {
        PopTrivialValues(instruction.push_pop.count);
        ++program_counter;
        break;
      }

      case vm::OpCode::COPY_TRIVIAL_VALUE: {
        const std::size_t destination_index = instruction.copy_trivial_value.destination;
        const std::size_t source_index = instruction.copy_trivial_value.source;
        // if (destination_index >= used_register_count_) {
        //   return Error("Destination out of bounds");
        // }
        // if (source_index >= used_register_count_) {
        //   return Error("Source out of bounds");
        // }
        ValueStorage::CopyTrivially(&registers_[destination_index], &registers_[source_index]);
        ++program_counter;
        break;
      }

      case vm::OpCode::PUSH_TRIVIAL_CONSTANT: {
        const std::size_t constant_index = instruction.push_trivial_constant.constant;
        // if (constant_index < constants.size()) {
          PushValue();
          ValueStorage::CopyTrivially(&top(), &constants[constant_index]);
          ++program_counter;
        // } else {
        //   return Error("Invalid constant index");
        // }
        break;
      }

      case vm::OpCode::CALL_NATIVE_FUNCTION: {
        const auto function_pointer = top().as<NativeFunction*>();
        PopTrivialValue();
        function_pointer(this);
        ++program_counter;
        break;
      }

      case vm::OpCode::JUMP: {
        program_counter += instruction.jump_data.offset;
        break;
      }

      case vm::OpCode::JUMP_IF_TRUE: {
        const auto condition = top().as<bool>();
        PopTrivialValue();
        program_counter += condition ? instruction.jump_data.offset : 1;
        break;
      }

      case vm::OpCode::JUMP_IF_FALSE: {
        const auto condition = top().as<bool>();
        PopTrivialValue();
        program_counter += condition ? 1 : instruction.jump_data.offset;
        break;
      }

      case vm::OpCode::SUBTRACT_NUMBERS: {
        assert(instruction.number_operation_data.result < used_register_count_);
        assert(instruction.number_operation_data.first < used_register_count_);
        assert(instruction.number_operation_data.second < used_register_count_);
        registers_[instruction.number_operation_data.result].as<double>() =
            registers_[instruction.number_operation_data.first].as<double>() -
            registers_[instruction.number_operation_data.second].as<double>();
        ++program_counter;
        break;
      }

      case vm::OpCode::MULTIPLY_NUMBERS: {
        assert(instruction.number_operation_data.result < used_register_count_);
        assert(instruction.number_operation_data.first < used_register_count_);
        assert(instruction.number_operation_data.second < used_register_count_);
        registers_[instruction.number_operation_data.result].as<double>() =
            registers_[instruction.number_operation_data.first].as<double>() *
            registers_[instruction.number_operation_data.second].as<double>();
        ++program_counter;
        break;
      }

      case vm::OpCode::IS_NUMBER_GREATER: {
        assert(instruction.number_operation_data.result < used_register_count_);
        assert(instruction.number_operation_data.first < used_register_count_);
        assert(instruction.number_operation_data.second < used_register_count_);
        registers_[instruction.number_operation_data.result].as<bool>() =
            registers_[instruction.number_operation_data.first].as<double>() >
            registers_[instruction.number_operation_data.second].as<double>();
        ++program_counter;
        break;
      }

      default:
        return Error("Invalid instruction opcode: {}", static_cast<uint32_t>(instruction.opcode));
    }
  }

  return Success;
}

}  // namespace ovis

