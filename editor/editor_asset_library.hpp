#pragma once

#include <set>
#include <optional>

#include <emscripten/fetch.h>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/file.hpp>
#include <ovis/core/json.hpp>

namespace ove {

class EditorAssetLibrary : public ovis::DirectoryAssetLibrary {
 public:
  EditorAssetLibrary(const std::string& directory);

  bool CreateAsset(const std::string& asset_id, const std::string& type,
                   const std::vector<std::pair<std::string, std::variant<std::string, ovis::Blob>>>& files) override;

  bool SaveAssetFile(const std::string& asset_id, const std::string& file_type,
                     std::variant<std::string, ovis::Blob> content) override;
  std::optional<ovis::Blob> Package();

  void UploadFile(const std::string& filename);

 private:
  std::set<std::string> files_to_upload_;
  ovis::Blob current_file_content_;
  bool is_currently_uploading_ = false;

  void UploadNextFile();
};

}  // namespace ove
