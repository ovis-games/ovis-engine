#include <ovis/vm/module.hpp>
#include <ovis/vm/type.hpp>

namespace ovis {

bool operator==(const TypeMemoryLayout& lhs, const TypeMemoryLayout& rhs) {
  return
    lhs.native_type_id == rhs.native_type_id && 
    lhs.is_constructible == rhs.is_constructible && 
    lhs.is_copyable == rhs.is_copyable && 
    lhs.alignment_in_bytes == rhs.alignment_in_bytes && 
    lhs.size_in_bytes == rhs.size_in_bytes && 
    lhs.construct->handle() == rhs.construct->handle() && 
    ((!lhs.copy && !rhs.copy) || (lhs.copy && rhs.copy && lhs.copy->handle() == rhs.copy->handle())) && 
    ((!lhs.destruct && !rhs.destruct) || (lhs.destruct && rhs.destruct && lhs.destruct->handle() == rhs.destruct->handle()));
}

bool operator!=(const TypeMemoryLayout& lhs, const TypeMemoryLayout& rhs) {
  return !(lhs == rhs);
}

Type::Type(TypeId id, TypeDescription description) : id_(id), description_(std::move(description)) {
  if (module()) {
    module()->AddType(id);
  }
}

Type::~Type() {
  if (module()) {
    module()->RemoveType(id());
  }
}

void Type::UpdateDescription(const TypeDescription& description) {
  // The memory layout is not allowed to change
  assert(description_.memory_layout == description.memory_layout);

  // A module may only be added but not changed!
  assert(!description_.module || description_.module == description.module);
  if (!description_.module && description.module) {
    description.module->AddType(id());
  }

  description_ = description;
}

std::string Type::GetReferenceString() const {
  if (name().length() == 0) {
    return "Unknown";
  } else if (module() == nullptr) {
    return std::string(name());
  } else {
    return fmt::format("{}.{}", module()->name(), name());
  }
}

bool Type::IsDerivedFrom(TypeId base_type_id) const {
  const Type* type = this;
  do {
    if (type->id() == base_type_id) {
      return true;
    }
    type = virtual_machine()->GetType(type->base_id());
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
    type = virtual_machine()->GetType(type->base_id());
  } while (type->id() != base_type_id);

  return pointer;
}

}  // namespace ovis
