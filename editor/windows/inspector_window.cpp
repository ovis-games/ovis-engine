#include "inspector_window.hpp"

#include "asset_editors/asset_editor.hpp"

namespace ovis {
namespace editor {

InspectorWindow::InspectorWindow() : ImGuiWindow("Inspector") {
  UpdateAfter("Dockspace Window");
  UpdateBefore("Overlay");
}

void InspectorWindow::DrawContent() {
  if (AssetEditor::last_focused_document_window != nullptr) {
    AssetEditor::last_focused_document_window->DrawInspectorContent();
  }
}

}  // namespace editor
}  // namespace ovis
