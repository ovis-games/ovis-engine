#pragma once

#include <type_traits>

#include <ovis/utils/json.hpp>

namespace ovis {

class Serializable {
 public:
  virtual ~Serializable() = default;

  virtual json Serialize() const = 0;
  virtual bool Deserialize(const json& data) = 0;
  virtual const json* GetSchema() const { return nullptr; }
};

}  // namespace ovis
