#include "packaging_window.hpp"

#include "../../editor_asset_library.hpp"

#include <imgui.h>

#include <ovis/core/json.hpp>
#include <ovis/core/log.hpp>
#include <ovis/engine/scene.hpp>

namespace ove {

PackagingWindow::PackagingWindow() : ModalWindow("PackagingWindow", "Package game") {
}

void PackagingWindow::DrawContent() {
  ImGui::Text("Version: ");

  ImGui::InputInt("Major", &version_.major, 1, 1);
  ImGui::InputInt("Minor", &version_.minor, 1, 1);
  ImGui::InputInt("Patch", &version_.patch, 1, 1);

  if (ImGui::Button("Package")) {
    static_cast<EditorAssetLibrary*>(ovis::GetApplicationAssetLibrary())->Package();
    Remove();
  }
  ImGui::SameLine();
  if (ImGui::Button("Cancel")) {
    Remove();
  }
}

}  // namespace ove
