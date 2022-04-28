#include <ovis/core/execution_context.hpp>

#include <ovis/utils/range.hpp>

namespace ovis {

ValueStorage& ExecutionContext::top(std::size_t offset) {
  assert(offset < used_register_count_);
  return registers_[used_register_count_ - (offset + 1)];
}


void ExecutionContext::PushUninitializedValues(std::size_t count) {
  // if (used_register_count_ + count > register_count_) {
  //   return Error("Stack overflow");
  // }
  // for (auto i : IRange(count)) {
  //   assert(registers_[used_register_count_ + i].native_type_id_ == TypeOf<void>);
  // }
  used_register_count_ += count;
  // return Success;
}


void ExecutionContext::PopTrivialValues(std::size_t count) {
  assert(count <= used_register_count_);
  for (auto i : IRange(count)) {
    registers_[used_register_count_ - (i + 1)].reset_trivial();
  }
  used_register_count_ -= count;
}

void ExecutionContext::PopValues(std::size_t count) {
  assert(count <= used_register_count_);
  for (auto i : IRange(count)) {
    registers_[used_register_count_ - (i + 1)].reset();
  }
  used_register_count_ -= count;
}

std::span<const ValueStorage> ExecutionContext::registers() const {
  return {registers_.get(), used_register_count_};
}

std::span<const ValueStorage> ExecutionContext::current_function_scope_registers() const {
  return {registers_.get(), used_register_count_};
}

}  // namespace ovis
