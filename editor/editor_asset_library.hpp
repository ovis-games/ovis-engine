#pragma once

#include <optional>
#include <set>

#include <emscripten/fetch.h>

#include <ovis/core/asset_library.hpp>
#include <ovis/utils/file.hpp>
#include <ovis/utils/json.hpp>

namespace ovis {
namespace editor {

class EditorAssetLibrary : public DirectoryAssetLibrary {
 public:
  EditorAssetLibrary(const std::string& directory);

  bool CreateAsset(const std::string& asset_id, const std::string& type,
                   const std::vector<std::pair<std::string, std::variant<std::string, Blob>>>& files) override;

  bool SaveAssetFile(const std::string& asset_id, const std::string& file_type,
                     std::variant<std::string, Blob> content) override;
  std::optional<Blob> Package();

  void UploadFile(const std::string& filename);

 private:
  std::set<std::string> files_to_upload_;
  Blob current_file_content_;
  bool is_currently_uploading_ = false;

  void UploadNextFile();
};

}  // namespace editor
}  // namespace ovis
