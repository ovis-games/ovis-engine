#include <ovis/core/module.hpp>
#include <ovis/core/type.hpp>

namespace ovis {

std::vector<Type::Registration> Type::registered_types = {
    {.id = Type::NONE_ID, .native_type_id = TypeOf<void>, .type = nullptr}};

Type::Type(TypeId id, std::shared_ptr<Module> module, TypeDescription description)
    : id_(id),
      module_(module),
      full_reference_(fmt::format("{}.{}", module->name(), description.name)),
      description_(std::move(description)) {}


bool Type::IsDerivedFrom(TypeId base_type_id) const {
  const Type* type = this;
  do {
    if (type->id() == base_type_id) {
      return true;
    }
    type = type->base();
  } while (type != nullptr);
  return false;
}

void* Type::CastToBase(TypeId base_type_id, void* pointer) const {
  assert(base_type_id != id());
  const Type* type = this;
  do {
    auto to_base_function = type->to_base_function();
    if (to_base_function == nullptr) {
      return nullptr;
    }

    auto result = to_base_function->Call<void*>(pointer);
    if (!result) {
      return nullptr;
    }
    pointer = *result;
    type = type->base();
  } while (type->id() != base_type_id);

  return pointer;
}

std::shared_ptr<Type> Type::Add(std::shared_ptr<Module> module, TypeDescription description) {
  for (auto& registration : registered_types) {
    if (registration.type == nullptr) {
      registration.id = registration.id.next();
      registration.native_type_id = description.memory_layout.native_type_id;
      return registration.type = std::shared_ptr<Type>(new Type(registration.id, module, std::move(description)));
    }
  }
  TypeId id(registered_types.size());
  registered_types.push_back({
    .id = id,
    .native_type_id = description.memory_layout.native_type_id,
    .type = std::shared_ptr<Type>(new Type(id, module, std::move(description))),
  });
  return registered_types.back().type;
}

Result<> Type::Remove(TypeId id) {
  if (id.index < registered_types.size() && registered_types[id.index].id == id) {
    registered_types[id.index].type = nullptr;
    return Success;
  } else {
    return Error("Invalid type id");
  }
}

std::shared_ptr<Type> Type::Deserialize(const json& data) {
  std::string_view module_name;
  std::string_view type_name;

  if (data.is_string()) {
    std::string_view type_string = data.get_ref<const std::string&>();
    auto period_position = type_string.find('.');
    if (period_position == std::string_view::npos) {
      return nullptr;
    }
    module_name = type_string.substr(0, period_position);
    type_name = type_string.substr(period_position + 1);
  } else if (data.is_object()) {
    if (!data.contains("module")) {
      return nullptr;
    }
    const auto& module_json = data.at("module");
    if (!module_json.is_string()) {
      return nullptr;
    }
    module_name = module_json.get_ref<const std::string&>();
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

  const std::shared_ptr<Module> module = Module::Get(module_name);
  if (module == nullptr) {
    return nullptr;
  }
  return module->GetType(type_name);
}

}  // namespace ovis
