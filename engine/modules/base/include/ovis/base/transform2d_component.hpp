#pragma once

#include <ovis/math/transform.hpp>
#include <ovis/engine/scene_object_component.hpp>

namespace ovis {

class Transform2DComponent : public SceneObjectComponent {
 public:
  std::vector<std::string> GetPropertyNames() const override;
  PropertyType GetPropertyType(const std::string& property_name) const override;
  PropertyValue GetProperty(const std::string& property_name) const override;
  void SetProperty(const std::string& property_name, const PropertyValue& value) override;

  inline Transform* transform() { return &transform_; }
  inline const Transform* transform() const { return &transform_; }

 private:
  Transform transform_;
};

}  // namespace ovis
