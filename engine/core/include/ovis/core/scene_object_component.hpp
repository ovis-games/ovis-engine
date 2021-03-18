#pragma once

#include <string>
#include <variant>
#include <vector>

#include <sol/sol.hpp>

#include <ovis/core/class.hpp>
#include <ovis/core/json.hpp>
#include <ovis/core/lua_reference.hpp>
#include <ovis/core/serialize.hpp>
#include <ovis/core/static_factory.hpp>
#include <ovis/math/vector.hpp>

namespace ovis {

class SceneObject;

class SceneObjectComponent : public Serializable,
                             public DynamicallyLuaReferencableBase,
                             public StaticFactory<SceneObjectComponent, std::unique_ptr<SceneObjectComponent>()> {
  MAKE_NON_COPY_OR_MOVABLE(SceneObjectComponent);
  friend class SceneObject;

 public:
  SceneObjectComponent() = default;
  virtual ~SceneObjectComponent() = default;

  inline SceneObject* scene_object() const { return scene_object_; }

  static void RegisterType(sol::table* module);

 private:
  SceneObject* scene_object_;
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

  const json* GetSchema() const override { return COMPONENT_SCHEMA; }
};

}  // namespace ovis
