#pragma once

#include "../../action_history.hpp"
#include "scene_view_editor.hpp"
#include <variant>

#include <ovis/core/scene.hpp>
#include <ovis/rendering/render_target_viewport.hpp>

namespace ovis {
namespace editor {

class SceneObjectEditor : public SceneViewEditor {
 public:
  SceneObjectEditor(const std::string& scene_object_asset);

  void DrawContent() override;
  void Save() override;

  static void CreateNew(const std::string& asset_id);

 protected:
  void SubmitChanges() override;
  void JsonFileChanged(const json& data, const std::string& file_type) override;
  void DrawObjectTree() override;
  void DrawAnimationPane();
  void DrawAnimationPaneObjectHierarchy(SceneObject* object);

  safe_ptr<SceneObject> object_;
};

}  // namespace editor
}  // namespace ovis
