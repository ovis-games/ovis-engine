#pragma once

#include <ovis/engine/scene.hpp>
#include <ovis/engine/scene_controller.hpp>

namespace ovis {
namespace editor {

class ObjectSelectionController : public SceneController {
 public:
  ObjectSelectionController(Scene* game_scene);

  void ProcessEvent(Event* event) override;

  void SelectObject(const std::string& object_name);
  void SelectObject(SceneObject* object);
  void ClearSelection();

  bool has_selected_object() const;
  const std::string& selected_object_name() const;
  SceneObject* selected_object() const;

 private:
  Scene* game_scene_;
  mutable std::string selected_object_name_;

  void CheckSelectionValidity() const;
};

}  // namespace editor
}  // namespace ovis
