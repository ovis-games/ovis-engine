#pragma once

#include <ovis/core/scene_controller.hpp>

namespace ovis::editor {

class SceneViewEditor;

class EditorController : public SceneController {
  friend class SceneViewEditor;

 public:
  inline EditorController(std::string_view name) : SceneController(name) {}

  inline Scene* game_scene() const { return game_scene_; }
  void SubmitChangesToScene();

 private:
  Scene* game_scene_;
  SceneViewEditor* editor_;
};

}  // namespace ovis::editor
