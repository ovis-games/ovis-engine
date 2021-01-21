#include "editor_module.hpp"

#include <imgui.h>

#include "editor_window_controller.hpp"
#include "windows/modal/loading_window.hpp"
#include "windows/modal/packaging_window.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <ovis/core/log.hpp>

#include <ovis/graphics/graphics_context.hpp>
#include <ovis/graphics/render_target_configuration.hpp>

#include <ovis/engine/render_pass.hpp>

namespace ove {

class ClearRenderPass : public ovis::RenderPass {
 public:
  ClearRenderPass() : ovis::RenderPass("ClearRenderPass") { RenderBefore("ImGui"); }

  void Render(ovis::Scene* scene) override {
    context()->default_render_target_configuration()->ClearColor(0, clear_color_);
  }

 private:
  glm::vec4 clear_color_;
};

EditorModule::EditorModule() : ovis::Module("Editor") {
  RegisterRenderPass("ClearRenderPass", [](ovis::Viewport*) { return std::make_unique<ClearRenderPass>(); });
  RegisterSceneController("EditorWindowController",
                          [this](ovis::Scene*) { return std::make_unique<EditorWindowController>(&log_history_); });
  RegisterSceneController("LoadingWindow", [this](ovis::Scene*) { return std::make_unique<LoadingWindow>(); });
  RegisterSceneController("PackagingWindow", [this](ovis::Scene*) { return std::make_unique<PackagingWindow>(); });

  ovis::Log::AddListener([this](ovis::LogLevel, const std::string& text) { log_history_.push_back(text); });
}

}  // namespace ove
