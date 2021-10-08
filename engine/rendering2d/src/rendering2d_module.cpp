
#include <ovis/core/core_module.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/rendering/rendering_module.hpp>
#include <ovis/rendering2d/rendering2d_module.hpp>
#include <ovis/rendering2d/shape2d.hpp>
#include <ovis/rendering2d/renderer2d.hpp>

namespace ovis {

int LoadRendering2DModule(lua_State* l) {
  sol::state_view state(l);

  /// This module provides 2D rendering components.
  // @module ovis.rendering2d
  // @usage local rendering2d = require('ovis.rendering2d')
  sol::table rendering2d_module = state.create_table();

  return rendering2d_module.push();
}

bool LoadRendering2DModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    LoadCoreModule();
    LoadRenderingModule();

    SceneObjectComponent::Register("Shape2D", [](SceneObject* object) { return std::make_unique<Shape2D>(object); });
    RenderPass::Register("Renderer2D", []() { return std::make_unique<Renderer2D>(); });

    lua.require("ovis.rendering2d", &LoadRendering2DModule);
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis
