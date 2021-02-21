#pragma once

#include <string>
#include <variant>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <ovis/core/class.hpp>
#include <ovis/core/json.hpp>
#include <ovis/core/serialize.hpp>

namespace ovis {

class SceneObjectComponent : public Serializable {
  MAKE_NON_COPY_OR_MOVABLE(SceneObjectComponent);

 public:
  SceneObjectComponent() = default;
  virtual ~SceneObjectComponent() = default;

  static std::vector<std::string> GetRegisteredComponents();
};

template <typename T, const json* COMPONENT_SCHEMA = nullptr>
class SimpleSceneObjectComponent : public SceneObjectComponent, public T {
public:
  json Serialize() const override {
    json data = static_cast<const T&>(*this);
    return data;
  }

  bool Deserialize(const json& data) override {
    try {
      static_cast<T&>(*this) = data;
      return true;
    } catch (...) {
      return false;
    }
  }

  const json* GetSchema() const override {
    return COMPONENT_SCHEMA;
  }
};

}  // namespace ovis
