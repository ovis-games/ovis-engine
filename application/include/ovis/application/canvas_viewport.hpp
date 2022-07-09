#pragma once

#if OVIS_EMSCRIPTEN

#include "ovis/rendering/rendering_viewport.hpp"

namespace ovis {

class CanvasViewport : public RenderingViewport {
 public:
  CanvasViewport(const char* target);
  ~CanvasViewport() override;

  std::string_view target() const { return target_; }

 private:
  std::string target_;
};

}  // namespace ovis

#endif
