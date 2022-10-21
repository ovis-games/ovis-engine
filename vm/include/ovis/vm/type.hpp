#pragma once

#include <memory>
#include <optional>
#include <span>
#include <type_traits>
#include <variant>
#include <vector>

#include "ovis/vm/attributes.hpp"
#include "ovis/vm/type_id.hpp"
#include "ovis/vm/type_memory_layout.hpp"
#include "ovis/vm/value.hpp"

namespace ovis {

class Function;

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
};

struct TypeDescription {
  VirtualMachine* virtual_machine;
  std::string module;
  std::string name;
  std::vector<TypePropertyDescription> properties;
  std::vector<std::shared_ptr<Function>> methods;
  TypeMemoryLayout memory_layout;
  Attributes attributes;
  std::unordered_map<std::string, Value> constants;

  // Implementations are in virtual_machine.hpp
  template <auto MEMBER_POINTER> requires(std::is_member_object_pointer_v<decltype(MEMBER_POINTER)>)
  void AddProperty(std::string_view name);

  template <auto GETTER> requires(!std::is_member_object_pointer_v<decltype(GETTER)>)
  void AddProperty(std::string_view name);

  template <auto GETTER, auto SETTER>
  void AddProperty(std::string_view name);

  template <auto METHOD>
  void AddMethod(std::string_view name);

  Result<> AddAttribute(std::string_view name, std::optional<Value> value = std::nullopt);
  template <typename T>
  Result<> AddAttribute(std::string_view name, T&& value);
};

class Type : public std::enable_shared_from_this<Type> {
 public:
  Type(TypeId id, TypeDescription description);

  static constexpr TypeId NONE_ID = TypeId(0);

  TypeId id() const { return id_; }
  const TypeDescription& description() const { return description_; }
  void UpdateDescription(const TypeDescription& description);

  std::string GetReferenceString() const;

  // Some helpers for accessing the description
  std::string_view name() const { return description().name; }
  std::string_view module() const { return description().module; }
  const TypeMemoryLayout& memory_layout() const { return description().memory_layout; }
  VirtualMachine* virtual_machine() const { return description().virtual_machine; }
  bool is_stored_inline() const;

  std::size_t alignment_in_bytes() const { return description().memory_layout.alignment_in_bytes; }
  std::size_t size_in_bytes() const { return description().memory_layout.size_in_bytes; }

  bool trivially_constructible() const { return description().memory_layout.construct == nullptr; }
  const Function* construct_function() const { return description().memory_layout.construct.get(); }

  bool trivially_copyable() const { return description().memory_layout.copy == nullptr; }
  const Function* copy_function() const { return description().memory_layout.copy.get(); }

  bool trivially_destructible() const { return description().memory_layout.destruct == nullptr; }
  const Function* destruct_function() const { return description().memory_layout.destruct.get(); }

  const Attributes& attributes() const { return description().attributes; }

  const TypePropertyDescription* GetProperty(std::string_view name) const;
  std::span<const TypePropertyDescription> properties() const { return description().properties; }

 private:
  TypeId id_;
  TypeDescription description_;
};

}  // namespace ovis
