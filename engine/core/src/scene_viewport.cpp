#include <ovis/core/scene_viewport.hpp>

namespace ovis {

void SceneViewport::RegisterType(sol::table* module) {
  /// A viewport renders the scene from a specific viewpoint.
  // @classmod ovis.core.SceneViewport
  sol::usertype<SceneViewport> scene_viewport = module->new_usertype<SceneViewport>("SceneViewport");

  /// The dimensions of the viewport (readonly).
  // @field[type=Vector2] dimensions
  scene_viewport["dimensions"] = sol::property(&SceneViewport::GetDimensions);
}

}  // namespace ovis