#pragma once

#include "../../action_history.hpp"
#include "asset_editor.hpp"
#include <variant>

#include <ovis/engine/render_target_viewport.hpp>
#include <ovis/engine/scene.hpp>

namespace ove {

class SceneEditor : public AssetEditor {
  enum class State { STOPPED, PAUSED, RUNNING };

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

  void CreateSceneViewport();

  void JsonFileChanged(const ovis::json& data, const std::string& file_type) override;

  State state_ = State::STOPPED;
  
  std::unique_ptr<ovis::RenderTargetViewport> scene_viewport_;
  bool scene_window_focused_ = false;

  ovis::Scene scene_;
  ovis::json serialized_scene_;

  struct SelectedScene {};
  struct SelectedObject {
    std::string name;
    static const SelectedObject NONE;
  };

  std::variant<SelectedScene, SelectedObject> selection_;
  std::vector<ovis::SceneObject*> cached_scene_objects_;
};

}  // namespace ove