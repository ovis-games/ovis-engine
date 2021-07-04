#pragma once

#include <optional>
#include <set>

#include <ovis/utils/file.hpp>
#include <ovis/utils/json.hpp>
#include <ovis/core/asset_library.hpp>

namespace ovis {
namespace editor {

class EditorAssetLibrary : public DirectoryAssetLibrary {
 public:
  EditorAssetLibrary(std::string_view directory);

  bool CreateAsset(std::string_view asset_id, std::string_view type,
                   const std::vector<std::pair<std::string, std::variant<std::string, Blob>>>& files) override;

  bool SaveAssetFile(std::string_view asset_id, std::string_view file_type,
                     std::variant<std::string, Blob> content) override;

  bool DeleteAsset(std::string_view asset_id) override;

  std::optional<Blob> Package();

  void UploadFile(std::string_view filename);

 private:
  std::set<std::string> files_to_upload_;
  Blob current_file_content_;
  bool is_currently_uploading_ = false;

  void UploadNextFile();
};

}  // namespace editor
}  // namespace ovis
