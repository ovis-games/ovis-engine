#include "loading_window.hpp"

#include "../../editor_asset_library.hpp"
#include "../../global.hpp"
#include <filesystem>
#include <fstream>

#include <imgui.h>

#include <ovis/utils/json.hpp>
#include <ovis/utils/log.hpp>
#include <ovis/core/scene.hpp>

namespace ovis {
namespace editor {

LoadingWindow::LoadingWindow() : ModalWindow("LoadingWindow", "Loading game assets") {
  if (!std::filesystem::exists("/assets")) {
    std::filesystem::create_directory("/assets");
  }

  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = &LoadingWindow::FileListDownloadSucceded;
  attr.onerror = &LoadingWindow::DownloadFailed;
  attr.userData = this;

  const std::string url = backend_url + "/v1/games/" + project_id + "/assetFiles";
  emscripten_fetch(&attr, url.c_str());
}

void LoadingWindow::DrawContent() {
  const float progress = downloaded_files_.size() / static_cast<float>(files_to_download_.size());
  ImGui::ProgressBar(progress, ImVec2(250, 0));
}

void LoadingWindow::FileListDownloadSucceded(emscripten_fetch_t* fetch) {
  std::string_view data(fetch->data, fetch->numBytes);
  try {
    json files = json::parse(data);

    auto controller = reinterpret_cast<LoadingWindow*>(fetch->userData);
    controller->files_to_download_ = files.get<std::vector<std::string>>();
    controller->DownloadNextFile();
  } catch (const json::parse_error& error) {
    LogE("Invalid json: {}", error.what());
  }
}

void LoadingWindow::FileDownloadSucceded(emscripten_fetch_t* fetch) {
  auto controller = reinterpret_cast<LoadingWindow*>(fetch->userData);
  LogD("Successfully downloaded '{}'", controller->current_file_);

  std::ofstream file(fmt::format("/assets/{}", controller->current_file_));
  SDL_assert(file.is_open());
  file.write(fetch->data, fetch->numBytes);
  file.close();

  controller->DownloadNextFile();
}

void LoadingWindow::DownloadFailed(emscripten_fetch_t* fetch) {
  const std::string error(fetch->data, fetch->data + fetch->numBytes);
  LogE("Failed to download {}:\n{}", fetch->url, error);
}

void LoadingWindow::DownloadNextFile() {
  if (current_file_ != "") {
    downloaded_files_.insert(current_file_);
  }
  if (files_to_download_.size() > 0) {
    current_file_ = files_to_download_.back();
    files_to_download_.pop_back();

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = &LoadingWindow::FileDownloadSucceded;
    attr.onerror = &LoadingWindow::DownloadFailed;
    attr.userData = this;

    const std::string url = backend_url + "/v1/games/" + project_id + "/assetFiles/" + current_file_;
    emscripten_fetch(&attr, url.c_str());

    LogD("Downloading '{}'", current_file_);
  } else {
    current_file_.clear();
    LogD("No more files to download");

    // This is okay as the callbacks are called from the main thread
    Remove();
    static_cast<EditorAssetLibrary*>(GetApplicationAssetLibrary())->Rescan();
  }
}

}  // namespace editor
}  // namespace ovis
