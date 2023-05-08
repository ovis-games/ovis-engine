#include "ovis/vm/type.hpp"

#include <cstring>

#include "ovis/utils/memory.hpp"
#include "ovis/vm/value_storage.hpp"

namespace ovis {

Type::Type(TypeId id, TypeDescription description) : id_(id), description_(std::move(description)) {}

void Type::UpdateDescription(const TypeDescription& description) {
  // The memory layout is not allowed to change
  assert(description_.memory_layout == description.memory_layout);

  // A module may only be added but not changed!
  assert(description_.module == "" || description_.module == description.module);

  description_ = description;
}

std::string Type::GetReferenceString() const {
  if (name().length() == 0) {
    return "Unknown";
  } else if (module() == "") {
    return std::string(name());
  } else {
    return fmt::format("{}.{}", module(), name());
  }
}

bool Type::is_stored_inline() const {
  return ValueStorage::IsTypeStoredInline(alignment_in_bytes(), size_in_bytes());
}

const TypePropertyDescription* Type::GetProperty(std::string_view name) const {
  for (const auto& property : properties()) {
    if (property.name == name) {
      return &property;
    }
  }
  return nullptr;
}

}  // namespace ovis
