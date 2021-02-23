#include <memory>

#include <ovis/engine/render_pass.hpp>

namespace ovis {

class ClearRenderPass : public RenderPass {
 public:
  ClearRenderPass();

  void Render(const RenderContext& render_context) override;

 private:
};

}  // namespace ovis
