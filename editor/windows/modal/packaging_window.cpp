#include "packaging_window.hpp"

#include "../../editor_asset_library.hpp"
#include "../../global.hpp"

#include <imgui.h>

#include <ovis/utils/json.hpp>
#include <ovis/utils/log.hpp>
#include <ovis/networking/fetch.hpp>
#include <ovis/core/scene.hpp>

namespace ovis {
namespace editor {

PackagingWindow::PackagingWindow() : ModalWindow("PackagingWindow", "Package game") {}

void PackagingWindow::DrawContent() {
  if (is_packaging_) {
    DrawProgress();
  } else {
    DrawConfiguration();
  }
}

void PackagingWindow::DrawConfiguration() {
  if (ImGui::BeginCombo("Version", version_.c_str())) {
    if (ImGui::Selectable("preview")) {
      version_ = "preview";
    }
    if (ImGui::Selectable("release")) {
      version_ = "release";
    }
    ImGui::EndCombo();
  }

  if (ImGui::Button("Package")) {
    is_packaging_ = true;
    auto package = static_cast<EditorAssetLibrary*>(GetApplicationAssetLibrary())->Package();
    progress_ = 0.5;

    if (package) {
      std::string url = fmt::format("{}/v1/games/{}/packages/{}", backend_url, project_id, version_);
      FetchOptions options;
      options.method = RequestMethod::PUT;
      options.headers["Content-Type"] = "application/octet-stream";
      options.on_success = [this](const FetchResponse&) {
        LogI("Successfully uploaded package.");
        Remove();
      };
      options.on_error = [this](const FetchResponse&) {
        LogE("Failed to upload package");
        Remove();
      };
      options.on_progress = [](const FetchProgress& progress) {
        LogD("File upload progress!!");
        // TODO: update loading bar (if possible?)
      };
      Fetch(url, options, std::move(*package));
    } else {
      Remove();
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Cancel")) {
    Remove();
  }
}

void PackagingWindow::DrawProgress() {
  ImGui::ProgressBar(progress_, ImVec2(250, 0));
}

}  // namespace editor
}  // namespace ovis
