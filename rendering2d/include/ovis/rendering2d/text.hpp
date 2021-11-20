#pragma once

#include <ovis/core/color.hpp>
#include <ovis/core/scene_object_component.hpp>

namespace ovis {

class Text : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();

 public:
  explicit Text(SceneObject* object);

  Color color() const { return color_; }
  std::string_view text() const { return text_; }
  std::string_view font() const { return font_; }

  json Serialize() const override;
  bool Deserialize(const json& data) override;
  const json* GetSchema() const override { return &schema; }

  static void RegisterType(vm::Module* module);

 private:
  Color color_;
  std::string text_;
  std::string font_;

  static const json schema;
};

}  // namespace ovis
