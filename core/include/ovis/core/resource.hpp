#pragma once

#include <vector>
#include <string>
#include "ovis/vm/type_id.hpp"

namespace ovis {

enum class ResourceAccess {
  NONE = 0,
  READ = 1,
  WRITE = 2,
  READ_WRITE = 3,
};


// struct ResourceAccess {
//   std::vector<std::string> names;
//   TypeId resource_type;
//   ResourceAccessType access_type;
//   bool is_group_resource;
// };

}  // namespace ovis
