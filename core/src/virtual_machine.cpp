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
//

VirtualMachine::VirtualMachine(std::size_t constants_capacity, std::size_t instruction_capacity)
    : constants_(std::make_unique<ValueStorage[]>(constants_capacity)),
      instructions_(std::make_unique<Instruction[]>(instruction_capacity)) {
  registered_types.push_back({.id = Type::NONE_ID, .native_type_id = TypeOf<void>});
}

std::shared_ptr<Module> VirtualMachine::RegisterModule(std::string_view name) {
  if (GetModule(name)) {
    return nullptr;
  }

  registered_modules_.push_back(std::make_shared<Module>(this, name));
  return registered_modules_.back();
}

void DeregisterModule(std::string_view name);

std::shared_ptr<Module> VirtualMachine::GetModule(std::string_view name) {
  for (const auto& module : registered_modules_) {
    if (module->name() == name) {
      return module;
    }
  }
  return nullptr;
}

TypeId VirtualMachine::GetTypeId(NativeTypeId native_type_id) {
  for (const auto& type_registration : registered_types) {
    if (type_registration.native_type_id == native_type_id) {
      return type_registration.id;
    }
  }
  const auto id = FindFreeTypeId();
  registered_types[id.index].native_type_id = native_type_id;
  return id;
}

Type* VirtualMachine::GetType(TypeId id) {
  assert(id.index < registered_types.size());
  return registered_types[id.index].id == id ? registered_types[id.index].type.get() : nullptr;
}

TypeId VirtualMachine::GetTypeId(const json& json) {
  assert(false && "Not implemented yet");
}

Type* VirtualMachine::GetType(const json& json) {
  assert(false && "Not implemented yet");
}

TypeId VirtualMachine::FindFreeTypeId() {
  for (const auto& type_registration : registered_types) {
    if (type_registration.native_type_id == TypeOf<void> && type_registration.type == nullptr &&
        type_registration.id != Type::NONE_ID) {
      return type_registration.id;
    }
  }
  TypeId id(registered_types.size());
  registered_types.push_back({ .id = id });
  return id;
}

}  // namespace ovis

