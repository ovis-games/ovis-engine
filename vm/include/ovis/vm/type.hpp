#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "ovis/utils/json.hpp"
#include "ovis/utils/native_type_id.hpp"
#include "ovis/utils/result.hpp"
#include "ovis/utils/versioned_index.hpp"
#include "ovis/utils/safe_pointer.hpp"
#include "ovis/vm/attributes.hpp"
#include "ovis/vm/type_helper.hpp"
#include "ovis/vm/type_id.hpp"
#include "ovis/vm/value_storage.hpp"

namespace ovis {

class Module;
class Function;
class Type;
class VirtualMachine;

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
  static TypePropertyDescription Create(VirtualMachine* virtual_machine, std::string_view name);

  static TypePropertyDescription Create(VirtualMachine* virtual_machine, std::string_view name,
                                        std::shared_ptr<Function> getter, std::shared_ptr<Function> setter = nullptr);
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

  // Constructs a single object
  Result<> Construct(void* memory) const;

  // Constructs count objects of the type for a given memory.
  // If an error occurs during construction, all constructed objects will be destructed.
  Result<> ConstructN(void* memory, std::size_t count) const;

  // Destructs an object. If an error occurs the program will be terminated.
  void Destruct(void* object) const;

  // Destructs count objects. If an error occurs it will terminate the program.
  void DestructN(void* objects, std::size_t count) const;

  // Copies a single object from source to destination
  Result<> Copy(void* destination, const void* source) const;

  // Copies count objects from source to destination. If an error occurs during copying the objects may be be partially copied.
  // Source and destination may not overlap (because of memcpy).
  Result<> CopyN(void* destination, const void* source, std::size_t count) const;

  template <typename T>
  static TypeMemoryLayout CreateForNativeType(VirtualMachine* virtual_machine);
};
bool operator==(const TypeMemoryLayout& lhs, const TypeMemoryLayout& rhs);
bool operator!=(const TypeMemoryLayout& lhs, const TypeMemoryLayout& rhs);

struct TypeReferenceDescription {
  TypeMemoryLayout memory_layout;
  std::shared_ptr<Function> get_pointer;
  std::shared_ptr<Function> set_pointer;

  template <typename T> static std::optional<TypeReferenceDescription> CreateFromNativeType(VirtualMachine* virtual_machine);
};

struct TypeDescription {
  VirtualMachine* virtual_machine;
  Module* module;
  std::string name;
  TypeId base;
  std::shared_ptr<Function> to_base;
  std::shared_ptr<Function> from_base;
  std::vector<TypePropertyDescription> properties;
  std::vector<std::shared_ptr<Function>> methods;
  TypeMemoryLayout memory_layout;
  std::optional<TypeReferenceDescription> reference;
  Attributes attributes;

  template <typename T, typename ParentType = void>
  requires (
    (std::is_same_v<ParentType, void> || std::is_base_of_v<ParentType, T>) &&
    (!std::is_same_v<T, ParentType> || std::is_same_v<T, void>)
  )
  static TypeDescription CreateForNativeType(VirtualMachine* virtual_machine, std::string_view name, Module* module = nullptr);

  template <typename T, typename ParentType = void>
  requires (
    (std::is_same_v<ParentType, void> || std::is_base_of_v<ParentType, T>) &&
    (!std::is_same_v<T, ParentType> || std::is_same_v<T, void>)
  )
  static TypeDescription CreateForNativeType(Module* module, std::string_view name);

  template <auto MEMBER_POINTER>
  requires (
    std::is_member_pointer_v<decltype(MEMBER_POINTER)> && !std::is_member_function_pointer_v<decltype(MEMBER_POINTER)>
  )
  void AddProperty(std::string_view name);

  template <auto GETTER>
  requires (
    std::is_function_v<decltype(GETTER)> ||
    (std::is_pointer_v<decltype(GETTER)> && std::is_function_v<std::remove_pointer_t<decltype(GETTER)>>) ||
    std::is_member_function_pointer_v<decltype(GETTER)>
  )
  void AddProperty(std::string_view name);

  template <auto GETTER, auto SETTER>
  void AddProperty(std::string_view name);

  template <auto METHOD>
  void AddMethod(std::string_view name);
};

class Type : public std::enable_shared_from_this<Type> {
  friend class Module;

 public:
  Type(TypeId id, TypeDescription description);
  ~Type();

  static constexpr TypeId NONE_ID = TypeId(0);

  TypeId id() const { return id_; }
  const TypeDescription& description() const { return description_; }
  void UpdateDescription(const TypeDescription& description);

  std::string GetReferenceString() const;

  // Some helpers for accessing the description
  std::string_view name() const { return description().name; }
  TypeId base_id() const { return description().base; }
  Module* module() const { return description().module; }
  bool is_reference_type() const { return description().reference.has_value(); }
  const TypeMemoryLayout& memory_layout() const { return description().memory_layout; }
  VirtualMachine* virtual_machine() const { return description().virtual_machine; }

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
  template <typename T> bool IsDerivedFrom() const;

  // Returns nullptr if the base_type_id is not actually a base of the type or if you pass in the id of the type itself
  // (so you cannot cast a type to "itself").
  void* CastToBase(TypeId base_type_id, void* pointer) const;
  template <typename Base>
  Base* CastToBase(void* pointer) const;

  const Attributes& attributes() const { return description().attributes; }

  const TypePropertyDescription* GetProperty(std::string_view name) const;
  std::span<const TypePropertyDescription> properties() const { return description().properties; }

  template <auto METHOD>
  void AddMethod(std::string_view name);

 private:
  TypeId id_;
  TypeDescription description_;
};

}  // namespace ovis

// Inline implementation
#include "ovis/utils/reflection.hpp"
#include "ovis/vm/function.hpp"
#include "ovis/vm/module.hpp"
#include "ovis/vm/virtual_machine.hpp"

namespace ovis {

template <auto PROPERTY> requires std::is_member_pointer_v<decltype(PROPERTY)>
inline TypePropertyDescription TypePropertyDescription::Create(VirtualMachine* virtual_machine, std::string_view name) {
  return {
    .name = std::string(name),
    .type = virtual_machine->GetTypeId<typename reflection::MemberPointer<PROPERTY>::MemberType>(),
    .access = PrimitiveAccess {
      .offset = reflection::MemberPointer<PROPERTY>::offset
    }
  };
}

inline TypePropertyDescription TypePropertyDescription::Create(VirtualMachine* virtual_machine, std::string_view name,
                                                               std::shared_ptr<Function> getter,
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
std::shared_ptr<Function> GetConstrucFunction(VirtualMachine* virtual_machine) {
  if constexpr (std::is_default_constructible_v<T>) {
    return Function::Create(
        FunctionDescription::CreateForNativeFunction<&type_helper::DefaultConstruct<T>>(virtual_machine));
  } else {
    return nullptr;
  }
}

template <typename T>
std::shared_ptr<Function> GetCopyFunction(VirtualMachine* virtual_machine) {
  if constexpr (std::is_copy_assignable_v<T> && !std::is_trivially_copy_assignable_v<T>) {
    return Function::Create(FunctionDescription::CreateForNativeFunction<&type_helper::CopyAssign<T>>(virtual_machine));
  } else {
    return nullptr;
  }
}

template <typename T>
std::shared_ptr<Function> GetDestructFunction(VirtualMachine* virtual_machine) {
  if constexpr (!std::is_trivially_destructible_v<T>) {
    return Function::Create(FunctionDescription::CreateForNativeFunction<&type_helper::Destruct<T>>(virtual_machine));
  } else {
    return nullptr;
  }
}

}  // namespace detail

template <typename T>
inline TypeMemoryLayout TypeMemoryLayout::CreateForNativeType(VirtualMachine* virtual_machine) {
  if constexpr (std::is_same_v<T, void>) {
    return {
      .native_type_id = TypeOf<T>,
      .is_constructible = false,
      .is_copyable = false,
      .alignment_in_bytes = 0,
      .size_in_bytes = 0,
      .construct = nullptr,
      .copy = nullptr,
      .destruct = nullptr,
    };
  } else {
    return {
      .native_type_id = TypeOf<T>,
      .is_constructible = std::is_default_constructible_v<T>,
      .is_copyable = std::is_copy_assignable_v<T>,
      .alignment_in_bytes = alignof(T),
      .size_in_bytes = sizeof(T),
      .construct = detail::GetConstrucFunction<T>(virtual_machine),
      .copy =  detail::GetCopyFunction<T>(virtual_machine),
      .destruct = detail::GetDestructFunction<T>(virtual_machine),
    };
  }
}

template <typename T>
std::optional<TypeReferenceDescription> TypeReferenceDescription::CreateFromNativeType(VirtualMachine* virtual_machine) {
  if constexpr (std::is_base_of_v<SafelyReferenceable, T>) {
    return TypeReferenceDescription {
      .memory_layout = TypeMemoryLayout::CreateForNativeType<safe_ptr<T>>(virtual_machine),
      .get_pointer = Function::Create(FunctionDescription::CreateForNativeFunction<&safe_ptr<T>::get>(virtual_machine)),
      .set_pointer = Function::Create(FunctionDescription::CreateForNativeFunction<&safe_ptr<T>::reset>(virtual_machine)),
    };
  } else {
    return std::nullopt;
  }
}

namespace detail {

template <typename T, typename ParentType> requires (!std::is_same_v<ParentType, void>)
std::shared_ptr<Function> CreateToBaseFunction(VirtualMachine* virtual_machine) {
  return Function::Create(
      FunctionDescription::CreateForNativeFunction<&type_helper::ToBase<ParentType, T>>(virtual_machine));
}

template <typename T, typename ParentType> requires (std::is_same_v<ParentType, void>)
std::shared_ptr<Function> CreateToBaseFunction(VirtualMachine* virtual_machine) {
  return nullptr;
}

template <typename T, typename ParentType> requires (!std::is_same_v<ParentType, void>)
std::shared_ptr<Function> CreateFromBaseFunction(VirtualMachine* virtual_machine) {
  return Function::Create(
      FunctionDescription::CreateForNativeFunction<&type_helper::FromBase<ParentType, T>>(virtual_machine));
}

template <typename T, typename ParentType> requires (std::is_same_v<ParentType, void>)
std::shared_ptr<Function> CreateFromBaseFunction(VirtualMachine* virtual_machine) {
  return nullptr;
}

}

template <typename T, typename ParentType> 
requires (
  (std::is_same_v<ParentType, void> || std::is_base_of_v<ParentType, T>) &&
  (!std::is_same_v<T, ParentType> || std::is_same_v<T, void>)
)
inline TypeDescription TypeDescription::CreateForNativeType(VirtualMachine* virtual_machine, std::string_view name, Module* module) {
  return TypeDescription{
    .virtual_machine = virtual_machine,
    .module = module,
    .name = std::string(name),
    .base = std::is_same_v<ParentType, void> ? Type::NONE_ID : virtual_machine->GetTypeId<ParentType>(),
    .to_base = detail::CreateToBaseFunction<T, ParentType>(virtual_machine),
    .from_base = detail::CreateFromBaseFunction<T, ParentType>(virtual_machine),
    .properties = {},
    .memory_layout = TypeMemoryLayout::CreateForNativeType<T>(virtual_machine),
    .reference = TypeReferenceDescription::CreateFromNativeType<T>(virtual_machine),
  };
}

template <typename T, typename ParentType>
requires (
  (std::is_same_v<ParentType, void> || std::is_base_of_v<ParentType, T>) &&
  (!std::is_same_v<T, ParentType> || std::is_same_v<T, void>)
)
inline TypeDescription TypeDescription::CreateForNativeType(Module* module, std::string_view name) {
  return CreateForNativeType<T, ParentType>(module->virtual_machine(), name, module);
}

template <auto MEMBER_POINTER>
requires (
  std::is_member_pointer_v<decltype(MEMBER_POINTER)> &&
  !std::is_member_function_pointer_v<decltype(MEMBER_POINTER)>
)
void TypeDescription::AddProperty(std::string_view name) {
  properties.push_back(TypePropertyDescription::Create<MEMBER_POINTER>(virtual_machine, name));
}

template <auto GETTER>
requires (
  std::is_function_v<decltype(GETTER)> ||
  (std::is_pointer_v<decltype(GETTER)> && std::is_function_v<std::remove_pointer_t<decltype(GETTER)>>) ||
  std::is_member_function_pointer_v<decltype(GETTER)>
)
void TypeDescription::AddProperty(std::string_view name) {
  properties.push_back(TypePropertyDescription::Create(
      virtual_machine, name,
      std::make_shared<Function>(FunctionDescription::CreateForNativeFunction<GETTER>(virtual_machine))));
}

template <auto GETTER, auto SETTER>
void TypeDescription::AddProperty(std::string_view name) {
  properties.push_back(TypePropertyDescription::Create(
      virtual_machine, name,
      std::make_shared<Function>(FunctionDescription::CreateForNativeFunction<GETTER>(virtual_machine)),
      std::make_shared<Function>(FunctionDescription::CreateForNativeFunction<SETTER>(virtual_machine))));
}

template <auto METHOD>
void TypeDescription::AddMethod(std::string_view name) {
  methods.push_back(
      std::make_shared<Function>(FunctionDescription::CreateForNativeFunction<METHOD>(virtual_machine, std::string(name))));
}

template <typename T>
bool Type::IsDerivedFrom() const {
  return IsDerivedFrom(virtual_machine()->GetTypeId<T>());
}
template <typename Base>
Base* Type::CastToBase(void* pointer) const {
  return reinterpret_cast<Base*>(CastToBase(virtual_machine()->GetTypeId<Base>(), pointer));
}

inline const TypePropertyDescription* Type::GetProperty(std::string_view name) const {
  for (const auto& property : description().properties) {
    if (property.name == name) {
      return &property;
    }
  }
  assert(false && "Property not found");
  return nullptr;
}

}  // namespace ovis
