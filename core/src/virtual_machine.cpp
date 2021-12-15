#include <ovis/core/virtual_machine.hpp>

namespace ovis {
namespace vm {

std::vector<Type::Registration> Type::registered_types = {
    {.vm_type_id = Type::NONE_ID, .native_type_id = TypeOf<void>, .type = nullptr}};
std::vector<std::shared_ptr<Module>> Module::modules;
ExecutionContext ExecutionContext::global;

Result<> ExecutionContext::Execute(std::span<const Instruction> instructions, std::span<const ValueStorage> constants) {
  std::size_t program_counter = 0;
  while (program_counter < instructions.size()) {
    Instruction instruction = instructions[program_counter];
    switch (static_cast<OpCode>(instruction.opcode)) {
      case OpCode::EXIT: {
        return Success;
      }

      case OpCode::PUSH: {
        PushValues(instruction.push_pop.count);
        ++program_counter;
        break;
      }

      case OpCode::POP: {
        PopValues(instruction.push_pop.count);
        ++program_counter;
        break;
      }

      case OpCode::POP_TRIVIAL: {
        PopTrivialValues(instruction.push_pop.count);
        ++program_counter;
        break;
      }

      case OpCode::COPY_TRIVIAL_VALUE: {
        const std::size_t destination_index = instruction.copy_trivial_value.destination;
        const std::size_t source_index = instruction.copy_trivial_value.source;
        // if (destination_index >= used_register_count_) {
        //   return Error("Destination out of bounds");
        // }
        // if (source_index >= used_register_count_) {
        //   return Error("Source out of bounds");
        // }
        ValueStorage::copy_trivially(&registers_[destination_index], &registers_[source_index]);
        ++program_counter;
        break;
      }

      case OpCode::PUSH_TRIVIAL_CONSTANT: {
        const std::size_t constant_index = instruction.push_trivial_constant.constant;
        // if (constant_index < constants.size()) {
          PushValue();
          ValueStorage::copy_trivially(&top(), &constants[constant_index]);
          ++program_counter;
        // } else {
        //   return Error("Invalid constant index");
        // }
        break;
      }

      case OpCode::CALL_NATIVE_FUNCTION: {
        const auto function_pointer = top().as<NativeFunction*>();
        PopTrivialValue();
        function_pointer(this);
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

      case OpCode::SUBTRACT_NUMBERS: {
        assert(instruction.number_operation_data.result < used_register_count_);
        assert(instruction.number_operation_data.first < used_register_count_);
        assert(instruction.number_operation_data.second < used_register_count_);
        registers_[instruction.number_operation_data.result].as<double>() =
            registers_[instruction.number_operation_data.first].as<double>() -
            registers_[instruction.number_operation_data.second].as<double>();
        ++program_counter;
        break;
      }

      case OpCode::MULTIPLY_NUMBERS: {
        assert(instruction.number_operation_data.result < used_register_count_);
        assert(instruction.number_operation_data.first < used_register_count_);
        assert(instruction.number_operation_data.second < used_register_count_);
        registers_[instruction.number_operation_data.result].as<double>() =
            registers_[instruction.number_operation_data.first].as<double>() *
            registers_[instruction.number_operation_data.second].as<double>();
        ++program_counter;
        break;
      }

      case OpCode::IS_NUMBER_GREATER: {
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

}  // namespace vm
}  // namespace ovis

