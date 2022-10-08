#pragma once

#if OVIS_EMSCRIPTEN

#include "emscripten/html5_webgl.h"

#include "ovis/utils/all.hpp"
#include "ovis/core/scene.hpp"
#include "ovis/graphics/graphics_context.hpp"
#include "ovis/rendering/rendering_viewport.hpp"

namespace ovis {

class CanvasViewport : public RenderingViewport, public All<CanvasViewport> {
 public:
  CanvasViewport(std::string target);
  ~CanvasViewport() override;

  std::string_view target() const { return target_; }

  Vector2 GetDimensions() const override;

  RenderTargetConfiguration* GetDefaultRenderTargetConfiguration() override;

  void Render() override;

 private:
  // Checks if the css dimensions have changed and resizes the framebuffer accordingly.
  void CheckForDimensionChanges();

  std::string target_;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context_;
  GraphicsContext graphics_context_;
};

}  // namespace ovis

#endif
