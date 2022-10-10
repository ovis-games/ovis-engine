#pragma once

#include "ovis/core/color.hpp"
#include "ovis/core/scene_object_component.hpp"
#include "ovis/core/vm_bindings.hpp"

namespace ovis {

class Text : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  Text();

  Color color() const { return color_; }
  std::string_view text() const { return text_; }
  std::string_view font() const { return font_; }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &schema; }

  OVIS_VM_DECLARE_TYPE_BINDING();

 private:
  Color color_;
  std::string text_;
  std::string font_;

  static const json schema;
};

}  // namespace ovis
