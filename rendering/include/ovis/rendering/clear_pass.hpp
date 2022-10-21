#pragma once

#include <optional>

#include "ovis/core/color.hpp"
#include "ovis/graphics/graphics_context.hpp"
#include "ovis/rendering/render_pass.hpp"

namespace ovis {

class ClearPass : public RenderPass {
 public:
  ClearPass(GraphicsContext* context, std::optional<Color> clear_color = std::nullopt);

  std::optional<ovis::Color> clear_color() const { return clear_color_; }
  void SetClearColor(Color color) { clear_color_ = color; }
  void DisableColorClearing() { clear_color_.reset(); }

  void Render(const SceneViewport&) override;

 private:
  std::optional<ovis::Color> clear_color_ = Color::Black();
  std::optional<float> clear_depth_ = 1.0f;
};

}  // namespace ovis
