#pragma once

#include <string_view>

#include "ovis/utils/safe_pointer.hpp"
#include "ovis/core/intersection.hpp"
#include "ovis/core/scene.hpp"
#include "ovis/core/scene_controller.hpp"
#include "ovis/editor/viewport_controller.hpp"

namespace ovis {
namespace editor {

class ObjectSelectionController : public ViewportController {
 public:
  static constexpr std::string_view Name() { return "ObjectSelectionController"; }

  ObjectSelectionController(EditorViewport* editor_viewport);

  void Update(std::chrono::microseconds) override;
  void ProcessEvent(Event* event) override;

  void SelectObject(std::string_view object_path);
  void SelectObject(SceneObject* object);
  void ClearSelection() { SelectObject(nullptr); }

  bool has_selected_object() const;
  SceneObject* selected_object() const;
  AxisAlignedBoundingBox3D selected_object_aabb() const { return selected_object_aabb_; }

 private:
  std::optional<std::string> selected_object_path_;
  AxisAlignedBoundingBox3D selected_object_aabb_;
};

}  // namespace editor
}  // namespace ovis
