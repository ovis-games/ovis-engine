#include <ovis/core/core_module.hpp>
#include <ovis/rendering/clear_pass.hpp>
#include <ovis/rendering/rendering_module.hpp>

namespace ovis {

bool LoadRenderingModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    LoadCoreModule();
    RenderPass::Register("ClearPass", []() { return std::make_unique<ClearPass>(); });
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis
