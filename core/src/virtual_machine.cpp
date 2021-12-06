#include <ovis/core/virtual_machine.hpp>

namespace ovis {
namespace vm {

std::vector<Type::Registration> Type::registered_types = {
    {.vm_type_id = Type::NONE_ID, .native_type_id = TypeOf<void>, .type = nullptr}};
std::vector<std::shared_ptr<Module>> Module::modules;
ExecutionContext ExecutionContext::global;

}  // namespace vm
}  // namespace ovis

