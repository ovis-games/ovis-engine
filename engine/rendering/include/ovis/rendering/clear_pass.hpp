#pragma once

#include <optional>

#include <ovis/core/color.hpp>
#include <ovis/rendering/render_pass.hpp>

namespace ovis {

class ClearPass : public RenderPass {
 public:
  ClearPass();

  void Render(const RenderContext&) override;

 private:
  std::optional<ovis::Color> clear_color_ = Color::Black();
  std::optional<float> clear_depth_ = 1.0f;
};

}  // namespace ovis
