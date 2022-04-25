#pragma once

#include <memory>
#include <variant>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/utils/native_type_id.hpp>
#include <ovis/utils/result.hpp>
#include <ovis/utils/versioned_index.hpp>
#include <ovis/core/type_helper.hpp>
#include <ovis/core/type_id.hpp>
#include <ovis/core/virtual_machine.hpp>

namespace ovis {

class Module;

struct TypePropertyDescription {
  struct PrimitiveAccess {
    std::uintptr_t offset;
  };
  struct FunctionAccess {
    std::shared_ptr<Function> getter;
    std::shared_ptr<Function> setter;
  };

  std::string name;
  TypeId type;
  std::variant<PrimitiveAccess, FunctionAccess> access;

  template <auto PROPERTY> requires std::is_member_pointer_v<decltype(PROPERTY)>
  static TypePropertyDescription Create(std::string_view name);

  static TypePropertyDescription Create(std::string_view name, std::shared_ptr<Function> getter,
                                        std::shared_ptr<Function> setter = nullptr);
};

// This structure defined the memory information of a type.
struct TypeMemoryLayout {
  NativeTypeId native_type_id;

  std::uint32_t is_constructible : 1;
  std::uint32_t is_copyable : 1;

  std::uintptr_t alignment_in_bytes;
  std::uintptr_t size_in_bytes;

  // If is_copy_constructible is true, this member must point to a function that constructs the type.
  std::shared_ptr<Function> construct;
  // If is_copyable is true, this member may point to a function that copies the type. Otherwise it is assumed the type
  // is trivially copyable.
  std::shared_ptr<Function> copy;
  // This member may point to a function that destructs the type. Otherwise, it is assumed the type is trivially
  // destructible.
  std::shared_ptr<Function> destruct;

  template <typename T>
  static TypeMemoryLayout CreateForNativeType();
};

struct TypeReferenceDescription {
  TypeMemoryLayout memory_layout;
  std::shared_ptr<Function> get_pointer;
  std::shared_ptr<Function> set_pointer;

  template <typename T> static std::optional<TypeReferenceDescription> CreateFromNativeType();
};

struct TypeDescription {
  std::string name;
  std::shared_ptr<Type> base;
  std::shared_ptr<Function> to_base;
  std::shared_ptr<Function> from_base;
  std::vector<TypePropertyDescription> properties;
  TypeMemoryLayout memory_layout;
  std::optional<TypeReferenceDescription> reference;

  template <typename T, typename ParentType = void>
  requires (
    (std::is_same_v<ParentType, void> || std::is_base_of_v<ParentType, T>) &&
    !std::is_same_v<T, ParentType>
  )
  static TypeDescription CreateForNativeType(std::string_view name);
};

class Type : public std::enable_shared_from_this<Type> {
  friend class Module;

 public:
  static constexpr TypeId NONE_ID = TypeId(0);

  TypeId id() const { return id_; }
  const TypeDescription& description() const { return description_; }

  std::string_view name() const { return description().name; }
  std::string_view full_reference() const { return full_reference_; }
  Type* base() const { return description().base.get(); }
  std::weak_ptr<Module> module() const { return module_; }
  bool is_reference_type() const { return description().reference.has_value(); }

  std::size_t alignment_in_bytes() const { return description().memory_layout.alignment_in_bytes; }
  std::size_t size_in_bytes() const { return description().memory_layout.size_in_bytes; }
  bool is_stored_inline() const { return ValueStorage::IsTypeStoredInline(alignment_in_bytes(), size_in_bytes()); }

  bool trivially_constructible() const { return description().memory_layout.construct == nullptr; }
  const Function* construct_function() const { return description().memory_layout.construct.get(); }

  bool trivially_copyable() const { return description().memory_layout.copy == nullptr; }
  const Function* copy_function() const { return description().memory_layout.copy.get(); }

  bool trivially_destructible() const { return description().memory_layout.destruct == nullptr; }
  const Function* destruct_function() const { return description().memory_layout.destruct.get(); }

  const Function* to_base_function() const { return description().to_base.get(); }
  const Function* from_base_function() const { return description().from_base.get(); }

  // Returns true if the type has a base with the specified id or if the type itself has the specified id.
  bool IsDerivedFrom(TypeId base_type_id) const;
  template <typename T> bool IsDerivedFrom() const { return IsDerivedFrom(*Type::GetId<T>()); }

  // Returns nullptr if the base_type_id is not actually a base of the type or if you pass in the id of the type itself
  // (so you cannot cast a type to "itself").
  void* CastToBase(TypeId base_type_id, void* pointer) const;
  template <typename Base>
  Base* CastToBase(void* pointer) const {
    return reinterpret_cast<Base*>(CastToBase(*Type::GetId<Base>(), pointer));
  }

  // // Returns nullptr if derived_type_id is not actually derived from the type.
  // void* CastToBase(Id base_type_id, void* pointer) const;
  // template <typename Base>
  // Base* CastToBase(void* pointer) const {
  //   return reinterpret_cast<Base*>(CastToBase(*Type::GetId<Base>(), pointer));
  // }

  const TypePropertyDescription* GetProperty(std::string_view name) const;
  std::span<const TypePropertyDescription> properties() const { return description().properties; }

  template <typename T> static std::shared_ptr<Type> Get();
  template <typename T> static std::optional<TypeId> GetId(); //  TODO: Remove optional here!
  static std::shared_ptr<Type> Get(TypeId id);

  static std::shared_ptr<Type> Deserialize(const json& data);

  // json Serialize() const;

 private:
  Type(TypeId id, std::shared_ptr<Module> module, TypeDescription description);
  static std::shared_ptr<Type> Add(std::shared_ptr<Module> module, TypeDescription description);
  static Result<> Remove(TypeId id);

  TypeId id_;
  std::string full_reference_;
  std::weak_ptr<Module> module_;
  TypeDescription description_;

  struct Registration {
    TypeId id;
    NativeTypeId native_type_id;
    std::shared_ptr<Type> type;
  };

  // static std::vector<std::pair<TypeId, std::weak_ptr<Type>>> type_associations;
  static std::vector<Registration> registered_types;
};

}  // namespace ovis

// Inline implementation
#include <ovis/utils/reflection.hpp>
#include <ovis/core/function.hpp>

namespace ovis {

// inline Type::Type(std::shared_ptr<Module> module, std::string_view name, std::shared_ptr<Type> base,
//                   ConversionFunction to_base, ConversionFunction from_base)
//     : Type(module, name) {
//   base_ = base;
//   from_base_ = from_base;
//   to_base_ = to_base;
// }

namespace detail {


}  // namespace detail

template <auto PROPERTY> requires std::is_member_pointer_v<decltype(PROPERTY)>
inline TypePropertyDescription TypePropertyDescription::Create(std::string_view name) {
  return {
    .name = std::string(name),
    .type = *Type::GetId<typename reflection::MemberPointer<PROPERTY>::MemberType>(),
    .access = PrimitiveAccess {
      .offset = reflection::MemberPointer<PROPERTY>::offset
    }
  };
}

inline TypePropertyDescription TypePropertyDescription::Create(std::string_view name, std::shared_ptr<Function> getter,
                                                        std::shared_ptr<Function> setter) {
  assert(getter != nullptr);
  assert(getter->outputs().size() == 1);
  if (setter != nullptr) {
    assert(setter->inputs().size() == 2);
    assert(setter->GetInput(1)->type == getter->GetOutput(0)->type);
  }

  return {
    .name = std::string(name),
    .type = getter->GetOutput(0)->type,
    .access = FunctionAccess {
      .getter = getter,
      .setter = setter,
    }
  };
}

namespace detail {

template <typename T>
std::shared_ptr<Function> GetConstrucFunction() {
  if constexpr (std::is_default_constructible_v<T>) {
    return Function::Create(FunctionDescription::CreateForNativeFunction<&type_helper::DefaultConstruct<T>>());
  } else {
    return nullptr;
  }
}

template <typename T>
std::shared_ptr<Function> GetCopyFunction() {
  if constexpr (std::is_copy_assignable_v<T> && !std::is_trivially_copy_assignable_v<T>) {
    return Function::Create(FunctionDescription::CreateForNativeFunction<&type_helper::CopyAssign<T>>());
  } else {
    return nullptr;
  }
}

template <typename T>
std::shared_ptr<Function> GetDestructFunction() {
  if constexpr (!std::is_trivially_destructible_v<T>) {
    return Function::Create(FunctionDescription::CreateForNativeFunction<&type_helper::Destruct<T>>());
  } else {
    return nullptr;
  }
}

}  // namespace detail

template <typename T>
inline TypeMemoryLayout TypeMemoryLayout::CreateForNativeType() {
  return {
    .native_type_id = TypeOf<T>,
    .is_constructible = std::is_default_constructible_v<T>,
    .is_copyable = std::is_copy_assignable_v<T>,
    .alignment_in_bytes = alignof(T),
    .size_in_bytes = sizeof(T),
    .construct = detail::GetConstrucFunction<T>(),
    .copy =  detail::GetCopyFunction<T>(),
    .destruct = detail::GetDestructFunction<T>(),
  };
}

template <typename T>
std::optional<TypeReferenceDescription> TypeReferenceDescription::CreateFromNativeType() {
  if constexpr (std::is_base_of_v<SafelyReferenceable, T>) {
    return TypeReferenceDescription {
      .memory_layout = TypeMemoryLayout::CreateForNativeType<safe_ptr<T>>(),
      .get_pointer = Function::Create(FunctionDescription::CreateForNativeFunction<&safe_ptr<T>::get>()),
      .set_pointer = Function::Create(FunctionDescription::CreateForNativeFunction<&safe_ptr<T>::reset>()),
    };
  } else {
    return std::nullopt;
  }
}

template <typename T, typename ParentType> 
requires (
  (std::is_same_v<ParentType, void> || std::is_base_of_v<ParentType, T>) &&
  !std::is_same_v<T, ParentType>
)
inline TypeDescription TypeDescription::CreateForNativeType(std::string_view name) {
  return TypeDescription{
      .name = std::string(name),
      .base = Type::Get<ParentType>(),
      .to_base =
          std::is_same_v<ParentType, void>
              ? nullptr
              : Function::Create(FunctionDescription::CreateForNativeFunction<&type_helper::ToBase<ParentType, T>>()),
      .from_base =
          std::is_same_v<ParentType, void>
              ? nullptr
              : Function::Create(FunctionDescription::CreateForNativeFunction<&type_helper::FromBase<ParentType, T>>()),
      .properties = {},
      .memory_layout = TypeMemoryLayout::CreateForNativeType<T>(),
      .reference = TypeReferenceDescription::CreateFromNativeType<T>(),
  };
}

// template <typename T, typename ParentType> requires(std::is_base_of_v<SafelyReferenceable, T> && !std::is_same_v<T, ParentType>)
// inline TypeDescription TypeDescription::CreateForNativeType(std::string_view name) {
//   return CreateForNativeType<T, ParentType>(name);
// }

// inline bool Type::IsDerivedFrom(std::shared_ptr<Type> type) const {
//   assert(type != nullptr);

//   if (type.get() == this) {
//     return true;
//   } else {
//     const auto base_type = base_.lock();
//     return base_type != nullptr && base_type->IsDerivedFrom(type);
//   }
// }

// template <typename T>
// inline bool Type::IsDerivedFrom() const {
//   return IsDerivedFrom(Get<T>());
// }

// inline void Type::RegisterConstructorFunction(std::shared_ptr<Function> constructor) {
//   assert(constructor);

//   // Output must be the current type
//   if (constructor->outputs().size() != 1 || constructor->outputs()[0].type.lock().get() != this) {
//     return;
//   }

//   // Check if there is already a constructor with these parameters registered.
//   for (const auto& weak_function_ptr : constructor_functions_) {
//     const auto function = weak_function_ptr.lock();
//     if (!function) {
//       // TODO: delete this
//       continue;
//     }
//     if (function->inputs().size() != constructor->inputs().size()) {
//       continue;
//     }
//     bool arguments_different = false;
//     for (auto i : IRange(function->inputs().size())) {
//       const auto function_input_type = function->inputs()[i].type.lock();
//       const auto constructor_input_type = constructor->inputs()[i].type.lock();
//       if (function_input_type != constructor_input_type) {
//         arguments_different = true;
//         break;
//       }
//     }
//     if (!arguments_different) {
//       return;
//     }
//   }

//   constructor_functions_.push_back(constructor);
// }

// template <typename... Args>
// inline Value Type::Construct(Args&&... args) const {
//   for (const auto& weak_function_ptr : constructor_functions_) {
//     const auto function = weak_function_ptr.lock();
//     if (!function) {
//       // TODO: delete function
//       continue;
//     }
//     if (function->IsCallableWithArguments<Args...>()) {
//       return function->Call<Value>(std::forward<Args>(args)...);
//     }
//   }

//   return Value::None();
// }

// inline Type::Id Type::Register(std::shared_ptr<Type> type, TypeId native_type_id) {
//   for (auto& registration : registered_types) {
//     if (registration.type == nullptr) {
//       registration.native_type_id = native_type_id;
//       registration.type = std::move(type);
//       return registration.vm_type_id = registration.vm_type_id.next();
//     }
//   }
//   Id id(registered_types.size());
//   registered_types.push_back({
//     .vm_type_id = id,
//     .native_type_id = native_type_id,
//     .type = std::move(type)
//   });
//   return id;
// }

// inline Result<> Type::Deregister(Type::Id id) {
//   assert(id.index() <= registered_types.size());
//   if (registered_types[id.index()].vm_type_id == id) {
//     registered_types[id.index()].type = nullptr;
//     return Success;
//   } else {
//     return Error("Invalid id");
//   }
// }

template <typename T>
inline std::shared_ptr<Type> Type::Get() {
  for (const auto& registration : registered_types) {
    if (registration.native_type_id == TypeOf<T>) {
      return registration.type;
    }
  }
  return nullptr;
}

template <typename T>
inline std::optional<TypeId> Type::GetId() {
  for (const auto& registration : registered_types) {
    if (registration.native_type_id == TypeOf<T>) {
      return registration.id;
    }
  }
  return std::nullopt;
}

inline std::shared_ptr<Type> Type::Get(TypeId id) {
  assert(id.index < registered_types.size());
  const auto& registration = registered_types[id.index];
  return registration.id == id ? registration.type : nullptr;
}


// inline void Type::RegisterProperty(std::string_view name, Type::Id type_id, Property::GetFunction getter,
//                                    Property::SetFunction setter) {
//   properties_.push_back({
//       .type_id = type_id,
//       .name = std::string(name),
//       .getter = getter,
//       .setter = setter,
//   });
// }

// namespace detail {

// template <auto PROPERTY>
// class PropertyCallbacks {};

// template <typename T, typename PropertyType, PropertyType T::*PROPERTY>
// struct PropertyCallbacks<PROPERTY> {
//   static Value PropertyGetter(const ovis::Value& object) { return Value::Create(object.Get<T>().*PROPERTY); }

//   static void PropertySetter(ovis::Value* object, const ovis::Value& property_value) {
//     assert(property_value.type_id() == Type::template GetId<PropertyType>());
//     object->Get<T>().*PROPERTY = property_value.Get<PropertyType>();
//   }

//   static void Register(Type* type, std::string_view name) {
//     assert(Type::GetId<PropertyType>());
//     type->RegisterProperty(name, *Type::GetId<PropertyType>(), &PropertyGetter, &PropertySetter);
//   }
// };

// template <auto GETTER>
// struct PropertyGetter;

// template <typename PropertyType, typename ContainingType, PropertyType (ContainingType::*GETTER)() const>
// struct PropertyGetter<GETTER> {
//   static std::shared_ptr<Type> property_type() { return Type::Get<PropertyType>(); }
//   static Type::Id property_type_id() {
//     assert(Type::GetId<PropertyType>());
//     return *Type::GetId<PropertyType>();
//   }

//   static std::shared_ptr<Type> containing_type() { return Type::Get<ContainingType>(); }

//   // template <FunctionPointerType GETTER>
//   static Value Get(const ovis::Value& object) { return Value::Create((object.Get<ContainingType>().*GETTER)()); }
// };

// template <auto GETTER>
// struct PropertySetter;

// template <typename PropertyType, typename ContainingType, void (ContainingType::*SETTER)(PropertyType T)>
// struct PropertySetter<SETTER> {
//   static std::shared_ptr<Type> property_type() { return Type::Get<PropertyType>(); }
//   static Type::Id property_type_id() {
//     assert(Type::GetId<PropertyType>());
//     return *Type::GetId<PropertyType>();
//   }

//   static std::shared_ptr<Type> containing_type() { return Type::Get<ContainingType>(); }

//   // template <FunctionPointerType GETTER>
//   static void Set(ovis::Value* object, const Value& property) {
//     return (object->Get<ContainingType>().*SETTER)(property.Get<PropertyType>());
//   }
// };

// }  // namespace detail

// template <auto PROPERTY>
// requires std::is_member_pointer_v<decltype(PROPERTY)>
// inline void Type::RegisterProperty(std::string_view name) {
//   // TODO: add assert that the class type is the same as the current type
//   const auto member_type_id = Type::GetId<typename MemberPointer<PROPERTY>::MemberType>();
//   assert(member_type_id);

//   properties_.push_back({
//     .type_id = *member_type_id,
//     .name = std::string(name),
//     .offset = MemberPointer<PROPERTY>::offset,
//   });
// }

// template <auto GETTER>
// inline void Type::RegisterProperty(std::string_view name) {
//   using GetterWrapper = detail::PropertyGetter<GETTER>;
//   RegisterProperty(name, GetterWrapper::property_type(), &GetterWrapper::PropertyGetter, nullptr);
// }

// template <auto GETTER, auto SETTER>
// inline void Type::RegisterProperty(std::string_view name) {
//   using GetterWrapper = detail::PropertyGetter<GETTER>;
//   using SetterWrapper = detail::PropertySetter<SETTER>;
//   assert(GetterWrapper::property_type() == SetterWrapper::property_type());
//   assert(GetterWrapper::containing_type() == SetterWrapper::containing_type());
//   assert(GetterWrapper::containing_type().get() == this);
//   RegisterProperty(name, GetterWrapper::property_type_id(), &GetterWrapper::Get, &SetterWrapper::Set);
// }

inline const TypePropertyDescription* Type::GetProperty(std::string_view name) const {
  for (const auto& property : description().properties) {
    if (property.name == name) {
      return &property;
    }
  }
  assert(false && "Property not found");
  return nullptr;
}

// inline Value Type::CreateValue(const json& data) const {
//   if (deserialize_function_) {
//     return deserialize_function_(data);
//   } else {
//     return Value::None();
//   }
// }

// inline json Type::Serialize() const {
//   json type = json::object();
//   type["name"] = std::string(name());
//   const auto base_type = base().lock();
//   if (base_type) {
//     type["base"] = std::string(base_type->full_reference());
//   }

//   json& properties = type["properties"] = json::object();
//   for (const auto& property : this->properties()) {
//     const auto property_type = Type::Get(property.type_id);
//     properties[property.name] = std::string(property_type->full_reference());
//   }

//   return type;
// }

}  // namespace ovis
