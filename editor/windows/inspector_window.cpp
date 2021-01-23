#include "inspector_window.hpp"

#include "asset_editors/asset_editor.hpp"

namespace ove {

InspectorWindow::InspectorWindow() : UiWindow("Inspector") {
  UpdateAfter("Dockspace Window");
}

void InspectorWindow::DrawContent() {
  if (AssetEditor::last_focused_document_window != nullptr) {
    AssetEditor::last_focused_document_window->DrawInspectorContent();
  }
}

}  // namespace ove
