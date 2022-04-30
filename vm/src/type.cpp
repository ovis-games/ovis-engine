#include <ovis/vm/module.hpp>
#include <ovis/vm/type.hpp>

namespace ovis {

// std::vector<Type::Registration> Type::registered_types = {
//     {.id = Type::NONE_ID, .native_type_id = TypeOf<void>, .type = nullptr}};

Type::Type(TypeId id, std::shared_ptr<Module> module, TypeDescription description)
    : id_(id),
      module_(module),
      full_reference_(module ? fmt::format("{}.{}", module->name(), description.name) : description.name),
      description_(std::move(description)),
      virtual_machine_(description_.virtual_machine) {}

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

// TypeId Type::GetId(NativeTypeId native_type_id) {
//   for (auto& registration : registered_types) {
//     if (registration.native_type_id == native_type_id) {
//       return registration.id;
//     }
//   }

//   const auto type_id = FindFreeTypeId();
//   registered_types[type_id.index].native_type_id = native_type_id;
//   registered_types[type_id.index].id = type_id;
//   return type_id;
// }

// TypeId Type::FindFreeTypeId() {
//   for (auto& registration : registered_types) {
//     if (registration.native_type_id == TypeOf<void> && registration.id != NONE_ID && registration.type == nullptr) {
//       // The entry is not used anymore
//       return registration.id.next();
//     }
//   }
//   TypeId id(registered_types.size());
//   registered_types.push_back(Registration{
//     .id = id,
//     .native_type_id = TypeOf<void>,
//     .type = nullptr,
//   });
//   return id;
// }

// std::shared_ptr<Type> Type::Add(std::shared_ptr<Module> module, TypeDescription description) {
//   const auto type_id = description.memory_layout.native_type_id != TypeOf<void>
//                            ? GetId(description.memory_layout.native_type_id)
//                            : FindFreeTypeId();
//   assert(registered_types[type_id.index].type == nullptr);
//   return registered_types[type_id.index].type =
//              std::shared_ptr<Type>(new Type(type_id, module, std::move(description)));
// }

// Result<> Type::Remove(TypeId id) {
//   if (id.index < registered_types.size() && registered_types[id.index].id == id) {
//     registered_types[id.index].type = nullptr;
//     registered_types[id.index].native_type_id = TypeOf<void>;
//     return Success;
//   } else {
//     return Error("Invalid type id");
//   }
// }

// std::shared_ptr<Type> Type::Deserialize(const json& data) {
// }

}  // namespace ovis
