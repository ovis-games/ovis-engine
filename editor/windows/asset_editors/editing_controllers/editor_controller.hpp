#pragma once

#include <ovis/core/scene_controller.hpp>

namespace ovis::editor {

class SceneViewEditor;

class EditorController : public SceneController {
  friend class SceneViewEditor;

 public:
  inline EditorController(std::string_view name) : SceneController(name) {}

  inline Scene* game_scene() const { return game_scene_; }
  inline Scene* editing_scene() const { return scene(); }
  void SubmitChangesToScene();

  inline void ClearTooltip() { tooltip_.clear(); }
  inline void SetTooltip(std::string_view text) { tooltip_ = text; }
  inline const char* tooltip() const { return tooltip_.c_str(); }
  inline bool has_tooltip() const { return !tooltip_.empty(); }

 private:
  Scene* game_scene_;
  SceneViewEditor* editor_;
  std::string tooltip_;
};

}  // namespace ovis::editor
