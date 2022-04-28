#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <ovis/core/virtual_machine_instructions.hpp>

namespace ovis {

// // Forward declarations
// class Type;
class Value;
// class Function;
// class Module;
// class ExecutionContext;
class ValueStorage;

class VirtualMachine {
 public:
  VirtualMachine(std::size_t constants_capacity = 1024, std::size_t instruction_capacity = 1024 * 1024);

  // Registers a new function with the specified instructions and constants
  std::size_t RegisterFunction(std::span<const vm::Instruction> instructions, std::span<const Value> constants);
  const vm::Instruction* GetInstructionPointer(std::size_t offset) const;
  std::span<ValueStorage> GetConstantRange(std::size_t offset, std::size_t count);

 private:
  std::unique_ptr<ValueStorage[]> constants_;
  std::unique_ptr<vm::Instruction[]> instructions_;
};

namespace vm {

// Allocates count instructions in the vm. The offset from the beginning is returned. A span of the allocated
// instructions can be returned via GetInstructionRange(AllocateInstructions(count), count).
std::size_t AllocateInstructions(std::size_t count);
std::span<Instruction> GetInstructionRange(std::size_t offset, std::size_t count);

std::size_t AllocateConstants(std::size_t count);
std::span<ValueStorage> GetConstantRange(std::size_t offset, std::size_t count);

}  // namespace vm

}  // namespace ovis

#include <ovis/core/value_storage.hpp>
#include <ovis/core/value.hpp>
