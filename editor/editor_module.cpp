#include "editor_module.hpp"

#include "windows/modal/loading_window.hpp"
#include "windows/modal/packaging_window.hpp"
#include "windows/asset_editors/scene_editor/scene_editor_render_pass.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <ovis/core/log.hpp>
#include <ovis/graphics/graphics_context.hpp>
#include <ovis/graphics/render_target_configuration.hpp>
#include <ovis/engine/render_pass.hpp>

namespace ovis {
namespace editor {

class ClearRenderPass : public RenderPass {
 public:
  ClearRenderPass() : RenderPass("ClearRenderPass") { RenderBefore("ImGui"); }

  void Render(const RenderContext& render_context) override {
    context()->default_render_target_configuration()->ClearColor(0, clear_color_);
  }

 private:
  vector4 clear_color_;
};

EditorModule::EditorModule() : Module("Editor") {
  RegisterRenderPass("ClearRenderPass", [](Viewport*) { return std::make_unique<ClearRenderPass>(); });
  RegisterRenderPass("SceneEditorRenderPass", [](Viewport*) { return std::make_unique<SceneEditorRenderPass>(nullptr); });
  RegisterSceneController("LoadingWindow", [this](Scene*) { return std::make_unique<LoadingWindow>(); });
  RegisterSceneController("PackagingWindow", [this](Scene*) { return std::make_unique<PackagingWindow>(); });

  Log::AddListener([this](LogLevel, const std::string& text) { log_history_.push_back(text); });
}

}  // namespace editor
}  // namespace ovis
