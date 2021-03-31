#pragma once

#include "editor_controller.hpp"

#include <ovis/core/intersection.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/scene_controller.hpp>

namespace ovis {
namespace editor {

class ObjectSelectionController : public EditorController {
 public:
  static constexpr std::string_view Name() { return "ObjectSelectionController"; }

  ObjectSelectionController();

  void Update(std::chrono::microseconds) override;
  void ProcessEvent(Event* event) override;

  void SelectObject(const std::string& object_name);
  void SelectObject(SceneObject* object);
  void ClearSelection();

  bool has_selected_object() const;
  const std::string& selected_object_name() const;
  SceneObject* selected_object() const;
  AxisAlignedBoundingBox3D selected_object_aabb() const { return selected_object_aabb_; }

 private:
  mutable std::string selected_object_name_;
  AxisAlignedBoundingBox3D selected_object_aabb_;

  void CheckSelectionValidity() const;
};

}  // namespace editor
}  // namespace ovis
