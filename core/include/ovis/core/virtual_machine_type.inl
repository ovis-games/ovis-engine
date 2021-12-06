namespace ovis {
namespace vm {

inline Type::Type(std::shared_ptr<Module> module, std::string_view name)
    : module_(module),
      name_(name),
      full_reference_(fmt::format("{}.{}", module->name(), name)),
      serialize_function_(nullptr),
      deserialize_function_(nullptr) {}

inline Type::Type(std::shared_ptr<Module> module, std::string_view name, std::shared_ptr<Type> parent,
                  ConversionFunction to_base, ConversionFunction from_base)
    : Type(module, name) {
  parent_ = parent;
  from_base_ = from_base;
  to_base_ = to_base;
}

inline bool Type::IsDerivedFrom(std::shared_ptr<Type> type) const {
  assert(type != nullptr);

  if (type.get() == this) {
    return true;
  } else {
    const auto parent_type = parent_.lock();
    return parent_type != nullptr && parent_type->IsDerivedFrom(type);
  }
}

template <typename T>
inline bool Type::IsDerivedFrom() const {
  return IsDerivedFrom(Get<T>());
}

inline void Type::RegisterConstructorFunction(std::shared_ptr<Function> constructor) {
  assert(constructor);

  // Output must be the current type
  if (constructor->outputs().size() != 1 || constructor->outputs()[0].type.lock().get() != this) {
    return;
  }

  // Check if there is already a constructor with these parameters registered.
  for (const auto& weak_function_ptr : constructor_functions_) {
    const auto function = weak_function_ptr.lock();
    if (!function) {
      // TODO: delete this
      continue;
    }
    if (function->inputs().size() != constructor->inputs().size()) {
      continue;
    }
    bool arguments_different = false;
    for (auto i : IRange(function->inputs().size())) {
      const auto function_input_type = function->inputs()[i].type.lock();
      const auto constructor_input_type = constructor->inputs()[i].type.lock();
      if (function_input_type != constructor_input_type) {
        arguments_different = true;
        break;
      }
    }
    if (!arguments_different) {
      return;
    }
  }

  constructor_functions_.push_back(constructor);
}

template <typename... Args>
inline Value Type::Construct(Args&&... args) const {
  for (const auto& weak_function_ptr : constructor_functions_) {
    const auto function = weak_function_ptr.lock();
    if (!function) {
      // TODO: delete function
      continue;
    }
    if (function->IsCallableWithArguments<Args...>()) {
      return function->Call<Value>(std::forward<Args>(args)...);
    }
  }

  return Value::None();
}

template <typename T>
inline std::shared_ptr<Type> Type::Get() {
  return GetWeak<T>().lock();
}

template <typename T>
inline std::weak_ptr<Type> Type::GetWeak() {
  // const auto type_index = std::type_index(typeid(T));
  // const auto type = std::lower_bound(type_associations.begin(), type_associations.end(), type_index, [](const auto& pair, const auto& type_index) {
  //       return pair.first < type_index;
  //     });
  // if (type != type_associations.end() && type->first == type_index) {
  //   return type->second.lock();
  // } else {
  //   return nullptr;
  // }
  for (const auto& type : type_associations) {
    if (type.first == TypeOf<T>) {
      return type.second;
    }
  }
  return {};
}

inline std::shared_ptr<Type> Type::Deserialize(const json& data) {
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

  const std::shared_ptr<vm::Module> module = Module::Get(module_name);
  if (module == nullptr) {
    return nullptr;
  }
  return module->GetType(type_name);
}

inline void Type::RegisterProperty(std::string_view name, std::shared_ptr<Type> type, Property::GetFunction getter,
                                   Property::SetFunction setter) {
  properties_.push_back({
      .type = type,
      .name = std::string(name),
      .getter = getter,
      .setter = setter,
  });
}

namespace detail {

template <auto PROPERTY>
class PropertyCallbacks {};

template <typename T, typename PropertyType, PropertyType T::*PROPERTY>
struct PropertyCallbacks<PROPERTY> {
  static Value PropertyGetter(const ovis::vm::Value& object) { return Value::Create(object.Get<T>().*PROPERTY); }

  static void PropertySetter(ovis::vm::Value* object, const ovis::vm::Value& property_value) {
    assert(property_value.type().lock() == Type::template Get<PropertyType>());
    object->Get<T>().*PROPERTY = property_value.Get<PropertyType>();
  }

  static void Register(Type* type, std::string_view name) {
    type->RegisterProperty(name, Type::Get<PropertyType>(), &PropertyGetter, &PropertySetter);
  }
};

template <auto GETTER>
struct PropertyGetter;

template <typename PropertyType, typename ContainingType, PropertyType (ContainingType::*GETTER)() const>
struct PropertyGetter<GETTER> {
  static std::shared_ptr<Type> property_type() { return Type::Get<PropertyType>(); }

  static std::shared_ptr<Type> containing_type() { return Type::Get<ContainingType>(); }

  // template <FunctionPointerType GETTER>
  static Value Get(const ovis::vm::Value& object) { return Value::Create((object.Get<ContainingType>().*GETTER)()); }
};

template <auto GETTER>
struct PropertySetter;

template <typename PropertyType, typename ContainingType, void (ContainingType::*SETTER)(PropertyType T)>
struct PropertySetter<SETTER> {
  static std::shared_ptr<Type> property_type() { return Type::Get<PropertyType>(); }

  static std::shared_ptr<Type> containing_type() { return Type::Get<ContainingType>(); }

  // template <FunctionPointerType GETTER>
  static void Set(ovis::vm::Value* object, const Value& property) {
    return (object->Get<ContainingType>().*SETTER)(property.Get<PropertyType>());
  }
};

}  // namespace detail

template <auto PROPERTY>
requires std::is_member_pointer_v<decltype(PROPERTY)>
inline void Type::RegisterProperty(std::string_view name) {
  detail::PropertyCallbacks<PROPERTY>::Register(this, name);
}

template <auto GETTER>
inline void Type::RegisterProperty(std::string_view name) {
  using GetterWrapper = detail::PropertyGetter<GETTER>;
  RegisterProperty(name, GetterWrapper::property_type(), &GetterWrapper::PropertyGetter, nullptr);
}

template <auto GETTER, auto SETTER>
inline void Type::RegisterProperty(std::string_view name) {
  using GetterWrapper = detail::PropertyGetter<GETTER>;
  using SetterWrapper = detail::PropertySetter<SETTER>;
  assert(GetterWrapper::property_type() == SetterWrapper::property_type());
  assert(GetterWrapper::containing_type() == SetterWrapper::containing_type());
  assert(GetterWrapper::containing_type().get() == this);
  RegisterProperty(name, GetterWrapper::property_type(), &GetterWrapper::Get, &SetterWrapper::Set);
}

inline const Type::Property* Type::GetProperty(std::string_view name) const {
  for (const auto& property : properties_) {
    if (property.name == name) {
      return &property;
    }
  }
  assert(false && "Property not found");
  return nullptr;
}

inline Value Type::CreateValue(const json& data) const {
  if (deserialize_function_) {
    return deserialize_function_(data);
  } else {
    return Value::None();
  }
}

inline json Type::Serialize() const {
  json type = json::object();
  type["name"] = std::string(name());
  const auto parent_type = parent().lock();
  if (parent_type) {
    type["parent"] = std::string(parent_type->full_reference());
  }

  json& properties = type["properties"] = json::object();
  for (const auto& property : this->properties()) {
    const auto property_type = property.type.lock();
    properties[property.name] = std::string(property_type->full_reference());
  }

  return type;
}

}  // namespace vm
}  // namespace ovis
