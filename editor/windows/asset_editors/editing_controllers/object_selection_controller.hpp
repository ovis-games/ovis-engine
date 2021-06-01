#pragma once

#include <string_view>

#include "editor_controller.hpp"

#include <ovis/utils/safe_pointer.hpp>
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

  void SelectObject(std::string_view object_path) { selected_object_.reset(game_scene()->GetObject(object_path)); }
  inline void SelectObject(SceneObject* object) { selected_object_.reset(object); }
  inline void ClearSelection() { selected_object_.reset(); }

  inline bool has_selected_object() const { return selected_object_; }
  inline SceneObject* selected_object() const { return selected_object_.get(); }
  AxisAlignedBoundingBox3D selected_object_aabb() const { return selected_object_aabb_; }

 private:
  safe_ptr<SceneObject> selected_object_;
  AxisAlignedBoundingBox3D selected_object_aabb_;
};

SceneObject* GetSelectedObject(Scene* editing_scene);

}  // namespace editor
}  // namespace ovis
