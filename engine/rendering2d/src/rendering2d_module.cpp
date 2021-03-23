
#include <ovis/core/core_module.hpp>
#include <ovis/rendering/rendering_module.hpp>
#include <ovis/rendering2d/rendering2d_module.hpp>
#include <ovis/rendering2d/sprite.hpp>
#include <ovis/rendering2d/sprite_renderer.hpp>

namespace ovis {

bool LoadRendering2DModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    LoadCoreModule();
    LoadRenderingModule();

    SceneObjectComponent::Register("Sprite", []() { return std::make_unique<Sprite>(); });
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis
