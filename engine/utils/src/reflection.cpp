#include <ovis/utils/reflection.hpp>

namespace ovis {
std::unordered_map<std::type_index, safe_ptr<Type>> Type::type_associations;
}

