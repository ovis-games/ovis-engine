#include <ovis/core/virtual_machine.hpp>

namespace ovis {
namespace vm {

std::vector<std::pair<TypeId, std::weak_ptr<Type>>> Type::type_associations;
std::vector<std::shared_ptr<Module>> Module::modules;
ExecutionContext ExecutionContext::global;

}  // namespace vm
}  // namespace ovis

