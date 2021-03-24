#pragma once

#include <string>

#include <ovis/rendering/debug_render_pass.hpp>

namespace ovis {
namespace editor {

class SelectedObjectBoundingBox : public DebugRenderPass {
 public:
  SelectedObjectBoundingBox(Scene* editing_scene);

  void Render(const RenderContext& render_context) override;

 private:
  Scene* editing_scene_;
};

}  // namespace editor
}  // namespace ovis
