#include <ovis/editor_viewport/viewport_controller.hpp>
#include <ovis/editor_viewport/editor_viewport.hpp>

namespace ovis {
namespace editor {

EditorViewport* ViewportController::viewport() const {
  return EditorViewport::instance();
}

}
}
