#include "ovis/core/scene_viewport.hpp"
#include "ovis/core/vm_bindings.hpp"

namespace ovis {

OVIS_VM_DEFINE_TYPE_BINDING(Core, SceneViewport) {
  SceneViewport_type->AddAttribute("Core.SceneComponent");

  SceneViewport_type->AddProperty<&SceneViewport::dimensions>("dimensions");
  SceneViewport_type->AddProperty<&SceneViewport::world_to_view>("worldToView");
  SceneViewport_type->AddProperty<&SceneViewport::view_to_world>("viewToWorld");
  SceneViewport_type->AddProperty<&SceneViewport::clip_to_view>("clipToView");
  SceneViewport_type->AddProperty<&SceneViewport::view_to_clip>("viewToClip");

  // TODO: expose methods
}

}  // namespace ovis
