#include <ovis/core/function.hpp>
#include <ovis/core/module.hpp>
#include <ovis/core/type.hpp>
#include <ovis/core/virtual_machine.hpp>

namespace ovis {

VirtualMachine vm;

// namespace vm {

// namespace {
// std::vector<Instruction> code;
// std::array<ValueStorage, 1024> data;
// std::size_t data_value_count = 0;
// }  // namespace

// std::size_t AllocateInstructions(std::size_t count) {
//   const std::size_t offset = code.size();
//   // TODO: Add "UNUSED INSTRUCTION"
//   code.resize(code.size() + count);
//   return offset;
// }

// std::span<Instruction> GetInstructionRange(std::size_t offset, std::size_t count) {
//   assert(offset + count <= code.size());
//   return { code.data() + offset, count };
// }

// std::size_t AllocateConstants(std::size_t count) {
//   assert(data_value_count + count <= data.size());
//   std::size_t offset = data_value_count;
//   data_value_count += count;
//   return offset;
// }

// std::span<ValueStorage> GetConstantRange(std::size_t offset, std::size_t count) {
//   assert(offset + count <= data.size());
//   return { data.data() + offset, count };
// }

// }  // namespace vm

inline std::shared_ptr<Module> VirtualMachine::GetModule(std::string_view name) {
  for (const auto& module : modules) {
    if (module->name() == name) {
      return module;
    }
  }
  return nullptr;
}

}  // namespace ovis

