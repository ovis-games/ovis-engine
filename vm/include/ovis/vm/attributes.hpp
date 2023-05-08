#pragma once

#include <set>
#include <string>

#include "ovis/vm/type_id.hpp"
#include "ovis/vm/value.hpp"

namespace ovis {

struct AttributeDescription {
  std::string name;
  std::string module;
  TypeId type;
  Value default_value;
};

using Attributes = std::map<std::string, Value>;

}  // namespace ovis
