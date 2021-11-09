#include "ovis/core/virtual_machine.hpp"

namespace ovis {
namespace vm {

inline Type::Type(Module* module, std::string_view name)
    : module_(module), name_(name), parent_(nullptr), serialize_function_(nullptr), deserialize_function_(nullptr) {}

inline Type::Type(Module* module, std::string_view name, Type* parent, ConversionFunction to_base,
                  ConversionFunction from_base)
    : Type(module, name) {
  parent_ = parent;
  from_base_ = from_base;
  to_base_ = to_base;
}

inline bool Type::IsDerivedFrom(safe_ptr<Type> type) const {
  if (type.get() == this) {
    return true;
  } else {
    return parent_ != nullptr && parent_->IsDerivedFrom(type);
  }
}

template <typename T>
inline bool Type::IsDerivedFrom() const {
  return IsDerivedFrom(Get<T>());
}

inline void Type::RegisterConstructorFunction(safe_ptr<Function> constructor) {
  if (!constructor) {
    return;
  }

  // Output must be the current type
  if (constructor->outputs().size() != 1 || constructor->outputs()[0].type != this) {
    return;
  }
  
  // Check if there is already a constructor with these parameters registered.
  for (const auto& function : constructor_functions_) {
    if (function->inputs().size() != constructor->inputs().size()) {
      continue;
    }
    bool arguments_different = false;
    for (auto i : IRange(function->inputs().size())) {
      if (function->inputs()[i].type != constructor->inputs()[i].type) {
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
  for (const auto& function : constructor_functions_) {
    if (function->IsCallableWithArguments<Args...>()) {
      return function->Call<Value>(std::forward<Args>(args)...);
    }
  }

  return Value::None();
}

template <typename T>
inline safe_ptr<Type> Type::Get() {
  if (auto it = type_associations.find(typeid(T)); it != type_associations.end()) {
    return it->second;
  } else {
    return nullptr;
  }
}

inline safe_ptr<Type> Type::Deserialize(const json& data) {
  if (!data.contains("module")) {
    return nullptr;
  }
  const auto& module_json = data.at("module");
  if (!module_json.is_string()) {
    return nullptr;
  }
  const safe_ptr<vm::Module> module = Module::Get(module_json.get_ref<const std::string&>());
  if (module == nullptr) {
    return nullptr;
  }
  if (!data.contains("name")) {
    return nullptr;
  }
  const auto& name_json = data.at("name");
  if (!name_json.is_string()) {
    return nullptr;
  }
  return module->GetType(name_json.get_ref<const std::string&>());
}

inline void Type::RegisterProperty(std::string_view name, Type* type, Property::GetFunction getter,
                                   Property::SetFunction setter) {
  properties_.push_back({
      .type = safe_ptr(type),
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
    assert(property_value.type() == Type::template Get<PropertyType>());
    object->Get<T>().*PROPERTY = property_value.Get<PropertyType>();
  }

  static void Register(Type* type, std::string_view name) {
    type->RegisterProperty(name, Type::Get<PropertyType>().get(), &PropertyGetter, &PropertySetter);
  }
};

}  // namespace detail

template <auto PROPERTY>
inline void Type::RegisterProperty(std::string_view name) {
  detail::PropertyCallbacks<PROPERTY>::Register(this, name);
}

inline Value Type::CreateValue(const json& data) {
  if (deserialize_function_) {
    return deserialize_function_(data);
  } else {
    return Value::None();
  }
}

}
}
