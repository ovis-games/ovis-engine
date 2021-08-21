#pragma once

#include <string>

#include <ovis/rendering/primitive_renderer.hpp>

namespace ovis {
namespace editor {

class SelectedObjectBoundingBox : public PrimitiveRenderer {
 public:
  static constexpr std::string_view Name() { return "SelectedObjectBoundingBox"; }

  SelectedObjectBoundingBox();

  void Render(const RenderContext& render_context) override;
};

}  // namespace editor
}  // namespace ovis
