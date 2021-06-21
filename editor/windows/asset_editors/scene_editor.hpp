#pragma once

#include "../../action_history.hpp"
#include "scene_view_editor.hpp"
#include <variant>

#include <ovis/rendering/render_target_viewport.hpp>
#include <ovis/core/scene.hpp>

namespace ovis {
namespace editor {

class SceneEditor : public SceneViewEditor {
 public:
  SceneEditor(const std::string& scene_asset);

  void Save() override;

  static void CreateNew(const std::string& asset_id);

 protected:
  void SubmitChanges() override;
  void JsonFileChanged(const json& data, const std::string& file_type) override;
  void DrawObjectTree() override;
};

}  // namespace editor
}  // namespace ovis