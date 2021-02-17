#pragma once

#include "modal_window.hpp"
#include <set>
#include <string>
#include <vector>

#include <emscripten/fetch.h>

namespace ovis {
namespace editor {

class LoadingWindow : public ModalWindow {
 public:
  LoadingWindow();

 private:
  void DrawContent() override;

  std::vector<std::string> files_to_download_;
  std::set<std::string> downloaded_files_;
  std::string current_file_;

  static void FileListDownloadSucceded(emscripten_fetch_t* fetch);
  static void FileDownloadSucceded(emscripten_fetch_t* fetch);
  static void DownloadFailed(emscripten_fetch_t* fetch);
  void DownloadNextFile();
};

}  // namespace editor
}  // namespace ovis
