#include <ovis/core/virtual_machine.hpp>

namespace ovis {
namespace vm {

std::unordered_map<std::type_index, safe_ptr<Type>> Type::type_associations;
std::vector<Module> Module::modules;
ExecutionContext ExecutionContext::global;

std::ostream& operator<<(std::ostream& os, const safe_ptr<Type>& pointer) {
  os << (pointer ? pointer->name() : "Unknown");
  return os;
}

}  // namespace vm
}  // namespace ovis

