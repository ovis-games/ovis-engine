#pragma once

#include "../../action_history.hpp"
#include "asset_editor.hpp"
#include "editing_controllers/editor_controller.hpp"

#include <ovis/rendering/render_target_viewport.hpp>
#include <ovis/core/scene.hpp>

namespace ovis {
namespace editor {

class SceneViewEditor : public AssetEditor {
  friend class EditorController;

 public:
  SceneViewEditor(const std::string& asset_id);

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessViewportInputEvent(Event* event);

  enum class RunState { STOPPED, PAUSED, RUNNING };
  inline RunState run_state() const { return run_state_; }
  void ChangeRunState(RunState new_state);

  inline Scene* game_scene() { return &game_scene_; }
  inline Scene* editing_scene() { return &editing_scene_; }

  SceneObject* GetSelectedObject();

 protected:
  void SetSerializedScene(const json& data);
  void SubmitChangesToScene();

  template <typename T>
  void AddEditorController() {
    static_assert(std::is_base_of_v<EditorController, T>);
    EditorController* controller = editing_scene()->AddController<T>();
    controller->game_scene_ = game_scene();
    controller->editor_ = this;
  }

  void CopySelectedObjectToClipboard();
  SceneObject* PasteObjectFromClipboard();

 private:
  void DrawContent() override;
  void DrawToolbar();
  void DrawViewport();

  void DrawInspectorContent() override;
  void DrawObjectTree();
  void DrawSelectionProperties();
  void DrawSceneProperties();
  void DrawSceneObjectProperties();

  void CreateSceneViewport(ImVec2 size);

  SceneObject* CreateObject(const std::string& base_name, bool initiate_rename = false);
  json::json_pointer GetComponentPath(const std::string& object_name, const std::string& component_id) {
    return json::json_pointer("/objects/" + object_name + "/components/" + component_id);
  }

  std::unique_ptr<RenderTargetViewport> scene_viewport_;

  Vector2 latest_mouse_position_ = Vector2::NotANumber();

  enum class RenamingState { IS_NOT_RENAMING, STARTED_RENAMING, IS_RENAMING };
  RenamingState renaming_state_ = RenamingState::IS_NOT_RENAMING;

  std::vector<SceneObject*> cached_scene_objects_;
  RunState run_state_ = RunState::STOPPED;
  json serialized_scene_editing_copy_;
  Scene game_scene_;
  Scene editing_scene_;
};

}  // namespace editor
}  // namespace ovis