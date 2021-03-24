#include <ovis/imgui/imgui_module.hpp>

namespace ovis {

bool LoadImGuiModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis