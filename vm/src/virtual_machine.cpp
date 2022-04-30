#include <ovis/vm/function.hpp>
#include <ovis/vm/module.hpp>
#include <ovis/vm/type.hpp>
#include <ovis/vm/virtual_machine.hpp>

namespace ovis {

// VirtualMachine vm;

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

VirtualMachine::VirtualMachine(std::size_t constant_capacity, std::size_t instruction_capacity, std::size_t main_execution_context_stack_size)
    : constants_(std::make_unique<ValueStorage[]>(constant_capacity)),
      instructions_(std::make_unique<Instruction[]>(instruction_capacity)),
      main_execution_context_(this, main_execution_context_stack_size) {
  registered_types_.push_back({
    .id = Type::NONE_ID,
    .native_type_id = TypeOf<void>,
    .type = std::make_shared<Type>(Type::NONE_ID, TypeDescription::CreateForNativeType<void>(this, "None"))
  });
  RegisterType(TypeDescription::CreateForNativeType<void*>(this, "MemoryAddress"));
  RegisterType(TypeDescription::CreateForNativeType<bool>(this, "Boolean"));
  RegisterType(TypeDescription::CreateForNativeType<double>(this, "Number"));
  RegisterType(TypeDescription::CreateForNativeType<std::string>(this, "String"));
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

  if (module_name.length() > 0) {
    const std::shared_ptr<Module> module = GetModule(module_name);
    if (module == nullptr) {
      return nullptr;
    }
    return module->GetType(type_name);
  } else {
    for (const auto& registered_type : registered_types_) {
      if (registered_type.type && registered_type.type->module() == nullptr && registered_type.type->name() == type_name) {
        return registered_type.type.get();
      }
    }
    return nullptr;
  }
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

