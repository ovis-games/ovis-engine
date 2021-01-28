#include <memory>

#include <ovis/engine/render_pass.hpp>

namespace ovis {

class ClearRenderPass : public RenderPass {
 public:
  ClearRenderPass();

  void Render(Scene* scene) override;

 private:
};

}  // namespace ovis
