#include "ovis/editor/viewport_controller.hpp"

#include "ovis/editor/editor_viewport.hpp"

namespace ovis {
namespace editor {

EditorViewport* ViewportController::viewport() const {
  return EditorViewport::instance();
}

}  // namespace editor
}  // namespace ovis
