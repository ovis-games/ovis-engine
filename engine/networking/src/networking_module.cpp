#include <ovis/networking/networking_module.hpp>

namespace ovis {

bool LoadNetworkingModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis