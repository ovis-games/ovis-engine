#pragma once

#include <any>
#include <limits>
#include <string_view>
#include <string>
#include <type_traits>
#include <ovis/utils/safe_pointer.hpp>

namespace ovis {

class ScriptContext;

struct ScriptTypeId {
  size_t value;
};
inline bool operator==(ScriptTypeId lhs, ScriptTypeId rhs) {
  return lhs.value == rhs.value;
}
static_assert(std::is_trivial_v<ScriptTypeId>);
constexpr ScriptTypeId SCRIPT_TYPE_UNKNOWN { 0 };

struct ScriptValue {
  ScriptTypeId type;
  std::any value;
};

struct ScriptType {
  using ConvertFunction = ScriptValue(*)(const ScriptValue& base_type_value);
  ScriptType(ScriptTypeId id, std::string_view name)
      : base_type_id(SCRIPT_TYPE_UNKNOWN),
        base_to_derived(nullptr),
        derived_to_base(nullptr),
        id(id),
        name(name) {}

  ScriptType(ScriptTypeId id, std::string_view name, ScriptTypeId base_type_id, ConvertFunction base_to_derived,
             ConvertFunction derived_to_base)
      : base_type_id(base_type_id),
        base_to_derived(base_to_derived),
        derived_to_base(derived_to_base),
        id(id),
        name(name) {
    assert(base_type_id != SCRIPT_TYPE_UNKNOWN);
    assert(base_to_derived != nullptr);
    assert(derived_to_base != nullptr);
  }

  ScriptTypeId base_type_id;
  ConvertFunction base_to_derived; 
  ConvertFunction derived_to_base; 
  ScriptTypeId id;
  std::string name;
};

struct ScriptValueDefinition {
  ScriptTypeId type;
  std::string identifier;
};

template <typename T>
concept Number = (std::is_integral_v<std::remove_reference_t<std::remove_cv_t<T>>> &&
                  !std::is_same_v<std::remove_reference_t<std::remove_cv_t<T>>, bool>) ||
                 std::is_floating_point_v<std::remove_reference_t<std::remove_cv_t<T>>>;

template <typename T>
concept ScriptReferenceType = std::is_base_of_v<SafelyReferenceable, T>;

template <typename T>
concept SafelyReferenceableObjectPointer =
    std::is_pointer_v<T> && std::is_base_of_v<SafelyReferenceable, std::remove_pointer_t<T>>;



}
