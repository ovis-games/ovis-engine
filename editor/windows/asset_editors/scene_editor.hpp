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
  ActionHistoryBase* GetActionHistory() override { return &action_history_; }

 private:
  void DrawContent() override;
  void DrawInspectorContent() override;

  void DrawObjectList();
  void DrawObjectComponentList();
  void DrawSceneProperties();

  void CreateSceneViewport();

  State state_ = State::STOPPED;
  std::unique_ptr<ovis::RenderTargetViewport> scene_viewport_;
  ovis::Scene scene_;
  ovis::json serialized_scene_;
  ActionHistory<ovis::Scene> action_history_;
  bool is_renaming_ = false;
  bool scene_window_focused_ = false;

  std::variant<ovis::Scene*, ovis::SceneObject*> selection_;
  std::vector<ovis::SceneObject*> cached_scene_objects_;
};

}  // namespace ove