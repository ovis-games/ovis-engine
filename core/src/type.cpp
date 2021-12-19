#include <ovis/core/module.hpp>
#include <ovis/core/type.hpp>

namespace ovis {

std::vector<Type::Registration> Type::registered_types = {
    {.vm_type_id = Type::NONE_ID, .native_type_id = TypeOf<void>, .type = nullptr}};

Type::Type(std::shared_ptr<Module> module, std::string_view name)
    : module_(module), name_(name), full_reference_(fmt::format("{}.{}", module->name(), name)) {}

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
