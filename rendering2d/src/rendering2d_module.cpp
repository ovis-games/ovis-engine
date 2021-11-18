
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
  static safe_ptr<vm::Module> rendering2d_module;
  if (!rendering2d_module) {
    LoadCoreModule();
    LoadRenderingModule();
    rendering2d_module = vm::Module::Register("Rendering2D");

    SceneObjectComponent::Register("Rendering2D.Shape2D", [](SceneObject* object) { return std::make_unique<Shape2D>(object); });
    RenderPass::Register("Renderer2D", []() { return std::make_unique<Renderer2D>(); });

    Shape2D::RegisterType(rendering2d_module.get());

    lua.require("ovis.rendering2d", &LoadRendering2DModule);
  }

  return true;
}

}  // namespace ovis
