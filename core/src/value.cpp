#include <ovis/core/value.hpp>

namespace ovis {

Result<Value> Value::Construct(std::shared_ptr<Type> type) {
  assert(type);

  Value value;
  void* stored_value_pointer = value.storage_.AllocateIfNecessary(type->alignment_in_bytes(), type->size_in_bytes());

  assert(type->construct_function());
  const auto constructor_result =
      ExecutionContext::global_context()->Call<void>(type->construct_function()->handle(), stored_value_pointer);
  OVIS_CHECK_RESULT(constructor_result);  // If the constructor failed the storage is freed by the destructor of value

  if (auto destructor = type->destruct_function(); destructor) {
    value.storage_.SetDestructFunction(destructor->handle());
  }

  value.type_ = std::move(type);

  return value;
}

}
