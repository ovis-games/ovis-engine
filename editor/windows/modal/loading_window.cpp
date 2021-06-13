#include "loading_window.hpp"

#include "../../editor_asset_library.hpp"
#include "../../global.hpp"
#include <filesystem>
#include <fstream>

#include <imgui.h>

#include <ovis/utils/json.hpp>
#include <ovis/utils/log.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/networking/fetch.hpp>

namespace ovis {
namespace editor {

LoadingWindow::LoadingWindow() : ModalWindow("LoadingWindow", "Loading game assets") {
  if (!std::filesystem::exists("/assets")) {
    std::filesystem::create_directory("/assets");
  }

  FetchOptions options;
  options.method = RequestMethod::GET;
  options.on_success = [this](const FetchResponse& response) {
    std::string_view data(reinterpret_cast<const char*>(response.body), response.content_length);
    try {
      json files = json::parse(data);

      files_to_download_ = files.get<std::vector<std::string>>();
      DownloadNextFile();
    } catch (const json::parse_error& error) {
      LogE("Invalid json: {}", error.what());
    }
  };
  options.on_error = [](const FetchResponse& response) {
    LogE("Failed downloading assets list!");
  };
  Fetch(fmt::format("{}/v1/games/{}/{}/assets/", backend_url, user_name, game_name), options);
}

void LoadingWindow::DrawContent() {
  const float progress = downloaded_files_.size() / static_cast<float>(files_to_download_.size());
  ImGui::ProgressBar(progress, ImVec2(250, 0));
}

void LoadingWindow::DownloadNextFile() {
  if (current_file_ != "") {
    downloaded_files_.insert(current_file_);
  }

  if (files_to_download_.size() > 0) {
    current_file_ = files_to_download_.back();
    files_to_download_.pop_back();

    FetchOptions options;
    options.method = RequestMethod::GET;
    options.on_success = [this](const FetchResponse& response) {
      LogD("Successfully downloaded '{}'", current_file_);

      std::ofstream file(fmt::format("/assets/{}", current_file_));
      SDL_assert(file.is_open());
      file.write(reinterpret_cast<const char*>(response.body), response.content_length);
      file.close();

      DownloadNextFile();
    };
    options.on_error = [this](const FetchResponse& response) {
      LogE("Failed downloading asset file {}!", current_file_);
    };
    Fetch(fmt::format("{}/v1/games/{}/{}/assets/{}", backend_url, user_name, game_name, current_file_), options);

    LogD("Downloading '{}'", current_file_);
  } else {
    current_file_.clear();
    LogD("No more files to download");

    static_cast<EditorAssetLibrary*>(GetApplicationAssetLibrary())->Rescan();

    // This is okay as the callbacks are called from the main thread
    Remove();
  }
}

}  // namespace editor
}  // namespace ovis
