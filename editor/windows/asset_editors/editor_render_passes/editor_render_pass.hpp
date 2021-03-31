#pragma once

#include <ovis/rendering/render_pass.hpp>

namespace ovis::editor {

class EditorRenderPass : public RenderPass {
 public:
  EditorRenderPass(std::string_view name);

  bool is_simulating() const;
};

}  // namespace ovis::editor
