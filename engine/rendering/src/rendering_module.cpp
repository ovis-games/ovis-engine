#include <ovis/rendering/rendering_module.hpp>
#include <ovis/core/core_module.hpp>
#include <ovis/math/math_module.hpp>

namespace ovis
{

bool LoadRenderingModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    LoadCoreModule();
    LoadMathModule();
    module_loaded = true;
  }

  return true;
}

} // namespace ovis
