#include "ovis/vm/virtual_machine.hpp"

#include "ovis/vm/function.hpp"
#include "ovis/vm/type.hpp"
#include "ovis/vm/value.hpp"

namespace ovis {

VirtualMachine::VirtualMachine(std::size_t constant_capacity, std::size_t instruction_capacity,
                               std::size_t main_execution_context_stack_size)
    : constants_(std::make_unique<ValueStorage[]>(constant_capacity)),
      constant_capacity_(constant_capacity),
      constant_count_(0),
      instructions_(std::make_unique<Instruction[]>(instruction_capacity)),
      instruction_capacity_(instruction_capacity),
      instruction_count_(0),
      main_execution_context_(this, main_execution_context_stack_size) {
  registered_types_.push_back({
    .id = Type::NONE_ID,
    .native_type_id = TypeOf<void>,
    .type = std::make_shared<Type>(Type::NONE_ID, TypeDescription {
      .virtual_machine = this,
      .module = "",
      .name = "None",
      .memory_layout = {
        .native_type_id = TypeOf<void>,
        .is_constructible = false,
        .is_copyable = false,
        .alignment_in_bytes = 0,
        .size_in_bytes = 0,
        .construct = nullptr,
        .copy = nullptr,
        .destruct = nullptr,
      }
    })
  });
  RegisterType<void*>("MemoryAddress", "");
  RegisterType<bool>("Boolean", "");
  RegisterType<double>("Number", "");
  RegisterType<std::string>("String", "");

  InsertInstructions(std::array{ Instruction::CreateHalt() });
}

VirtualMachine::~VirtualMachine() {
  for (auto i : IRange(constant_count_)) {
    constants_.get()[i].Reset(main_execution_context());
  }
}

std::size_t VirtualMachine::InsertInstructions(std::span<const Instruction> instructions) {
  assert(instruction_count_ + instructions.size() <= instruction_capacity_);
  const auto offset = instruction_count_;
  std::memcpy(instructions_.get() + offset, instructions.data(), instructions.size_bytes());
  instruction_count_ += instructions.size();
  return offset;
}

const Instruction* VirtualMachine::GetInstructionPointer(std::size_t offset) const {
  assert(offset < instruction_count_);
  return instructions_.get() + offset;
}

std::size_t VirtualMachine::InsertConstants(std::span<const Value> constants) {
  assert(constant_count_ + constants.size() <= constant_capacity_);
  const auto offset = constant_count_;
  for (auto i : IRange(constants.size())) {
    constants[i].CopyTo(constants_.get() + offset + i);
  }
  constant_count_ += constants.size();
  return offset;
}

const ValueStorage* VirtualMachine::GetConstantPointer(std::size_t offset) const {
  assert(offset == 0 || offset < constant_count_);
  return constants_.get() + offset;
}

Result<> VirtualMachine::RegisterModule(std::string_view name) {
  return registered_modules_.emplace(name).second ? Result<>(Success) : Error("Module {} already registered", name);
}

Result<> VirtualMachine::DeregisterModule(std::string_view name) {
  return registered_modules_.erase(std::string(name)) > 0 ? Result<>(Success)
                                                          : Error("Module {} is not registered", name);
}

bool VirtualMachine::IsModuleRegistered(std::string_view name) {
  return registered_modules_.contains(std::string(name));
}

Type* VirtualMachine::RegisterType(TypeDescription description) {
  assert(description.name.length() > 0);
  assert(description.memory_layout.native_type_id == TypeOf<void> ||
         GetType(description.memory_layout.native_type_id) == nullptr ||
         GetType(description.memory_layout.native_type_id)->name().length() == 0);

  const auto type_id = description.memory_layout.native_type_id != TypeOf<void>
                           ? GetTypeId(description.memory_layout.native_type_id)
                           : FindFreeTypeId();
  if (registered_types_[type_id.index].type) {
    registered_types_[type_id.index].type->UpdateDescription(description);
  } else {
    registered_types_[type_id.index].type = std::shared_ptr<Type>(new Type(type_id, std::move(description)));
  }
  return registered_types_[type_id.index].type.get();
}

Result<> VirtualMachine::DeregisterType(TypeId type_id) {
  if (type_id.index < registered_types_.size() && registered_types_[type_id.index].id == type_id) {
    registered_types_[type_id.index].id = registered_types_[type_id.index].id.next();
    registered_types_[type_id.index].type = nullptr;
    registered_types_[type_id.index].native_type_id = TypeOf<void>;
    return Success;
  } else {
    return Error("Invalid type id");
  }
}

Result<> VirtualMachine::DeregisterType(NotNull<Type*> type) {
  return DeregisterType(type->id());
}

TypeId VirtualMachine::GetTypeId(NativeTypeId native_type_id) {
  for (const auto& type_registration : registered_types_) {
    if (type_registration.native_type_id == native_type_id) {
      return type_registration.id;
    }
  }
  const auto id = FindFreeTypeId();
  registered_types_[id.index].native_type_id = native_type_id;
  return id;
}

TypeId VirtualMachine::GetTypeId(const json& data) {
  const auto type = GetType(data);
  return type ? type->id() : Type::NONE_ID;
}

Type* VirtualMachine::GetType(TypeId id) {
  assert(id.index < registered_types_.size());
  return registered_types_[id.index].id == id ? registered_types_[id.index].type.get() : nullptr;
}

Type* VirtualMachine::GetType(NativeTypeId native_type_id) {
  return GetType(GetTypeId(native_type_id));
}

Type* VirtualMachine::GetType(const json& data) {
  std::string_view module_name;
  std::string_view type_name;

  if (data.is_string()) {
    std::string_view type_string = data.get_ref<const std::string&>();
    auto period_position = type_string.find('.');
    if (period_position == std::string_view::npos) {
      type_name = type_string;
    } else {
      module_name = type_string.substr(0, period_position);
      type_name = type_string.substr(period_position + 1);
    }
  } else if (data.is_object()) {
    if (data.contains("module")) {
      const auto& module_json = data.at("module");
      if (!module_json.is_string()) {
        return nullptr;
      }
      module_name = module_json.get_ref<const std::string&>();
    }
    if (!data.contains("name")) {
      return nullptr;
    }
    const auto& name_json = data.at("name");
    if (!name_json.is_string()) {
      return nullptr;
    }
    type_name = name_json.get_ref<const std::string&>();
  } else {
    return nullptr;
  }

  for (const auto& type : registered_types_) {
    if (type.type && type.type->name() == type_name && type.type->module() == module_name) {
      return type.type.get();
    }
  }

  return nullptr;
}

Function* VirtualMachine::RegisterFunction(FunctionDescription description) {
  registered_functions_.push_back(Function::Create(description));
  return registered_functions_.back().get();
}

Function* VirtualMachine::GetFunction(std::string_view function_reference) {
  for (const auto& function : registered_functions_) {
    if (function->GetReferenceString() == function_reference) {
      return function.get();
    }
  }
  return nullptr;
}

TypeId VirtualMachine::FindFreeTypeId() {
  for (const auto& type_registration : registered_types_) {
    if (type_registration.native_type_id == TypeOf<void> && type_registration.type == nullptr &&
        type_registration.id != Type::NONE_ID) {
      return type_registration.id;
    }
  }
  TypeId id(registered_types_.size());
  registered_types_.push_back({ .id = id, .native_type_id = TypeOf<void> });
  return id;
}

}  // namespace ovis

