#pragma once

#include <memory>
#include <variant>
#include <vector>

#include <ovis/utils/json.hpp>
#include <ovis/utils/result.hpp>
#include <ovis/utils/type_id.hpp>
#include <ovis/utils/versioned_index.hpp>
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
  std::shared_ptr<Type> type;
  std::variant<PrimitiveAccess, FunctionAccess> access;
};

struct TypeDescription {
  TypeId native_type_id;
  std::string name;
  std::uintptr_t alignment_in_bytes;
  std::uintptr_t size_in_bytes;
  std::shared_ptr<Type> parent;
  std::vector<TypePropertyDescription> properties;

  // Default constructor
  std::shared_ptr<Function> construct;

  // If one of the following functions is null, it is assumed that the
  // type ist trivially copy/move-constrctible/assignable.
  std::shared_ptr<Function> copy_construct;
  std::shared_ptr<Function> move_construct;
  std::shared_ptr<Function> copy_assign;
  std::shared_ptr<Function> move_assign;

  // If the destruct function is null it is assumed it is trivially
  // destructible.
  std::shared_ptr<Function> destruct;

  template <typename T, typename ParentType = void> requires (!std::is_base_of_v<SafelyReferenceable, T>)
  static TypeDescription CreateForNativeType(std::string_view name);

  template <typename T, typename ParentType = void> requires (std::is_base_of_v<SafelyReferenceable, T>)
  static TypeDescription CreateForNativeType(std::string_view name);
};

class Type : public std::enable_shared_from_this<Type> {
  friend class Value;
  friend class Module;

 public:
  using Id = VersionedIndex<uint32_t, 20>;
  static constexpr Id NONE_ID = Id(0);

  Id id() const { return id_; }
  const TypeDescription& description() const { return description_; }

  std::string_view name() const { return description().name; }
  std::string_view full_reference() const { return full_reference_; }
  std::weak_ptr<Type> parent() const { return description().parent; }
  std::weak_ptr<Module> module() const { return module_; }

  std::size_t alignment_in_bytes() const { return description().alignment_in_bytes; }
  std::size_t size_in_bytes() const { return description().size_in_bytes; }

  bool trivially_constructible() const { return description().construct == nullptr; }
  const Function* construct_function() const { return description().construct.get(); }

  bool trivially_copy_constructible() const { return description().copy_construct == nullptr; }
  const Function* copy_construct_function() const { return description().copy_construct.get(); }

  bool trivially_move_constructible() const { return description().move_construct == nullptr; }
  const Function* move_construct_function() const { return description().move_construct.get(); }

  bool trivially_copy_assignable() const { return description().copy_assign == nullptr; }
  const Function* copy_assign_function() const { return description().copy_assign.get(); }

  bool trivially_move_assignable() const { return description().move_assign == nullptr; }
  const Function* move_assign_function() const { return description().move_assign.get(); }

  bool trivially_destructible() const { return description().destruct == nullptr; }
  const Function* destruct_function() const { return description().destruct.get(); }

  // bool IsDerivedFrom(std::shared_ptr<Type> type) const;
  // template <typename T> bool IsDerivedFrom() const;

  // void RegisterConstructorFunction(std::shared_ptr<Function> function);
  // template <typename... Args> Value Construct(Args&&... args) const;

  // void SetSerializeFunction(SerializeFunction function) { serialize_function_ = function; }
  // SerializeFunction serialize_function() const { return serialize_function_; }

  // void SetDeserializeFunction(DeserializeFunction function) { deserialize_function_ = function; }
  // DeserializeFunction deserialize_function() const { return deserialize_function_; }

  // Value CreateValue(const json& data) const;

  // void RegisterProperty(std::string_view name, Id type_id, Property::GetFunction getter,
  //                       Property::SetFunction setter = nullptr);
  // template <auto PROPERTY> requires std::is_member_pointer_v<decltype(PROPERTY)>
  // void RegisterProperty(std::string_view);

  // template <auto GETTER>
  // void RegisterProperty(std::string_view);

  // template <auto GETTER, auto SETTER>
  // void RegisterProperty(std::string_view);

  const TypePropertyDescription* GetProperty(std::string_view name) const;
  std::span<const TypePropertyDescription> properties() const { return description().properties; }

  template <typename T> static std::shared_ptr<Type> Get();
  template <typename T> static std::optional<Id> GetId();
  static std::shared_ptr<Type> Get(Id id);

  static std::shared_ptr<Type> Deserialize(const json& data);

  // json Serialize() const;

 private:
  Type(Id id, std::shared_ptr<Module> module, TypeDescription description);
  static std::shared_ptr<Type> Add(std::shared_ptr<Module> module, TypeDescription description);
  static Result<> Remove(Id id);

  Id id_;
  std::string full_reference_;
  std::weak_ptr<Module> module_;
  TypeDescription description_;

  struct Registration {
    Id id;
    TypeId native_type_id;
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

// inline Type::Type(std::shared_ptr<Module> module, std::string_view name, std::shared_ptr<Type> parent,
//                   ConversionFunction to_base, ConversionFunction from_base)
//     : Type(module, name) {
//   parent_ = parent;
//   from_base_ = from_base;
//   to_base_ = to_base;
// }

namespace detail {

template <typename T>
Result<> DefaultConstruct(ExecutionContext* context) {
  auto destination = context->top(1).as<void*>();
  auto source = context->top(0).as<const void*>();
  assert(reinterpret_cast<std::uintptr_t>(source) % alignof(T) == 0);
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignof(T) == 0);
  new (destination) T();
  return Success;
}

template <typename T>
Result<> CopyConstruct(ExecutionContext* context) {
  auto destination = context->top(1).as<void*>();
  auto source = context->top(0).as<const void*>();
  assert(reinterpret_cast<std::uintptr_t>(source) % alignof(T) == 0);
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignof(T) == 0);
  new (destination) T(*reinterpret_cast<const T*>(source));
  return Success;
}

template <typename T>
Result<> MoveConstruct(ExecutionContext* context) {
  auto destination = context->top(1).as<void*>();
  auto source = context->top(0).as<void*>();
  assert(reinterpret_cast<std::uintptr_t>(source) % alignof(T) == 0);
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignof(T) == 0);
  new (destination) T(std::move(*reinterpret_cast<T*>(source)));
  return Success;
}

template <typename T>
Result<> CopyAssign(ExecutionContext* context) {
  auto destination = context->top(1).as<void*>();
  auto source = context->top(0).as<const void*>();
  assert(reinterpret_cast<std::uintptr_t>(source) % alignof(T) == 0);
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignof(T) == 0);
  *reinterpret_cast<T*>(destination) = *reinterpret_cast<const T*>(source);
  return Success;
}

template <typename T>
Result<> MoveAssign(ExecutionContext* context) {
  auto destination = context->top(1).as<void*>();
  auto source = context->top(0).as<void*>();
  assert(reinterpret_cast<std::uintptr_t>(source) % alignof(T) == 0);
  assert(reinterpret_cast<std::uintptr_t>(destination) % alignof(T) == 0);
  *reinterpret_cast<T*>(destination) = std::move(*reinterpret_cast<T*>(source));
  return Success;
}

template <typename T>
Result<> Destruct(ExecutionContext* context) {
  auto value = context->top(0).as<void*>();
  reinterpret_cast<T*>(value)->~T();
  return Success;
}

}  // namespace detail

template <typename T, typename ParentType> requires(!std::is_base_of_v<SafelyReferenceable, T>)
inline TypeDescription TypeDescription::CreateForNativeType(std::string_view name) {
  static_assert(std::is_copy_constructible_v<T>);
  static_assert(std::is_move_constructible_v<T>);
  static_assert(std::is_copy_assignable_v<T>);
  static_assert(std::is_move_assignable_v<T>);
  return TypeDescription{
      .native_type_id = TypeOf<T>,
      .name = std::string(name),
      .alignment_in_bytes = alignof(T),
      .size_in_bytes = sizeof(T),
      .parent = Type::Get<ParentType>(),
      .properties = {},
      .construct =
          std::is_trivially_constructible_v<T> ? nullptr : Function::MakeNative(detail::DefaultConstruct<T>, {{}}, {}),
      .copy_construct = std::is_trivially_copy_constructible_v<T>
                            ? nullptr
                            : Function::MakeNative(detail::CopyConstruct<T>, {{}, {}}, {}),
      .move_construct = std::is_trivially_move_constructible_v<T>
                            ? nullptr
                            : Function::MakeNative(detail::MoveConstruct<T>, {{}, {}}, {}),
      .copy_assign =
          std::is_trivially_copy_assignable_v<T> ? nullptr : Function::MakeNative(detail::CopyAssign<T>, {{}, {}}, {}),
      .move_assign =
          std::is_trivially_move_assignable_v<T> ? nullptr : Function::MakeNative(detail::MoveAssign<T>, {{}, {}}, {}),
      .destruct = std::is_trivially_destructible_v<T> ? nullptr : Function::MakeNative(detail::MoveAssign<T>, {{}}, {}),
  };
}

template <typename T, typename ParentType> requires(std::is_base_of_v<SafelyReferenceable, T>)
inline TypeDescription TypeDescription::CreateForNativeType(std::string_view name) {
  return CreateForNativeType<safe_ptr<T>, ParentType>(name);
}

// inline bool Type::IsDerivedFrom(std::shared_ptr<Type> type) const {
//   assert(type != nullptr);

//   if (type.get() == this) {
//     return true;
//   } else {
//     const auto parent_type = parent_.lock();
//     return parent_type != nullptr && parent_type->IsDerivedFrom(type);
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
inline std::optional<Type::Id> Type::GetId() {
  for (const auto& registration : registered_types) {
    if (registration.native_type_id == TypeOf<T>) {
      return registration.id;
    }
  }
  return std::nullopt;
}

inline std::shared_ptr<Type> Type::Get(Id id) {
  assert(id.index() < registered_types.size());
  const auto& registration = registered_types[id.index()];
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
//   const auto parent_type = parent().lock();
//   if (parent_type) {
//     type["parent"] = std::string(parent_type->full_reference());
//   }

//   json& properties = type["properties"] = json::object();
//   for (const auto& property : this->properties()) {
//     const auto property_type = Type::Get(property.type_id);
//     properties[property.name] = std::string(property_type->full_reference());
//   }

//   return type;
// }

}  // namespace ovis
