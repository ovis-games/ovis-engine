#pragma once

#include <type_traits>

#include <ovis/utils/json.hpp>

namespace ovis {

class Serializable {
 public:
  virtual ~Serializable() = default;

  // Serializes the state of the object to json.
  virtual json Serialize() const = 0;

  // Recreates the state of the object from a previously serialized object.
  // The previously overservable state of the object should not influence the
  // deserialization.
  virtual bool Deserialize(const json& data) = 0;

  // Partially updates an object with serialized data. The difference to Deserialize()
  // is that the object gets "updated" not "recreated".
  virtual bool Update(const json& data) { return false; }

  // Returns the scema of the serialized json.
  virtual const json* GetSchema() const { return nullptr; }
};

}  // namespace ovis
