#pragma once

#include "../../action_history.hpp"
#include "asset_editor.hpp"
#include <variant>

#include <ovis/engine/render_target_viewport.hpp>
#include <ovis/engine/scene.hpp>

namespace ovis {
namespace editor {

class SceneEditor : public AssetEditor {
  enum class State { STOPPED, PAUSED, RUNNING };
  enum class RenamingState { IS_NOT_RENAMING, STARTED_RENAMING, IS_RENAMING };
  enum class EditingMode { MOVE, ROTATE, SCALE };

  struct MoveState {
    vector2 original_position;
    vector2 drag_start_mouse_position;
  };

 public:
  SceneEditor(const std::string& scene_asset);

  void Update(std::chrono::microseconds delta_time) override;
  bool ProcessEvent(const SDL_Event& event) override;
  // void DrawPropertyWindows() override;
  void Save() override;

 private:
  void DrawContent() override;
  void DrawInspectorContent() override;

  bool DrawObjectList();
  bool DrawObjectComponentList();

  void CreateSceneViewport(ImVec2 size);

  SceneObject* CreateObject(const std::string& base_name);
  SceneObject* GetSelectedObject();
  SceneObject* GetObjectAtPosition(vector2 world_position);

  void JsonFileChanged(const json& data, const std::string& file_type) override;

  State state_ = State::STOPPED;

  std::unique_ptr<RenderTargetViewport> scene_viewport_;
  bool scene_window_focused_ = false;

  Scene scene_;
  json serialized_scene_;

  struct SelectedScene {};
  struct SelectedObject {
    std::string name;
    static const SelectedObject NONE;
  };
  RenamingState renaming_state_ = RenamingState::IS_NOT_RENAMING;

  Camera camera_;
  Transform camera_transform_;

  std::variant<SelectedScene, SelectedObject> selection_;
  std::vector<SceneObject*> cached_scene_objects_;

  struct Icons {
    std::unique_ptr<Texture2D> play;
    std::unique_ptr<Texture2D> pause;
    std::unique_ptr<Texture2D> stop;
    std::unique_ptr<Texture2D> move;
    std::unique_ptr<Texture2D> rotate;
    std::unique_ptr<Texture2D> scale;
  } icons_;

  EditingMode editing_mode_ = EditingMode::MOVE;
  MoveState move_state_;
};

}  // namespace editor
}  // namespace ovis