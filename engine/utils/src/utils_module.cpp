#include <ovis/utils/utils_module.hpp>

namespace ovis {


bool LoadUtilsModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis
