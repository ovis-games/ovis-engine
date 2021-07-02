#pragma once

#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <variant>

#include <ovis/utils/file.hpp>
#include <ovis/utils/json.hpp>

namespace ovis {

class AssetLibrary {
 public:
  virtual ~AssetLibrary() = default;

  virtual bool Contains(std::string_view asset_id) const = 0;
  virtual std::vector<std::string> GetAssets() const = 0;
  virtual std::string GetAssetType(std::string_view asset_id) const = 0;
  virtual std::vector<std::string> GetAssetFileTypes(std::string_view asset_id) const = 0;
  virtual std::optional<std::string> LoadAssetTextFile(std::string_view asset_id,
                                                       std::string_view filename) const = 0;
  virtual std::optional<Blob> LoadAssetBinaryFile(std::string_view asset_id, std::string_view filename) const = 0;
  virtual std::vector<std::string> GetAssetsWithType(std::string_view type) const = 0;

  virtual bool CreateAsset(std::string_view asset_id, std::string_view type,
                           const std::vector<std::pair<std::string, std::variant<std::string, Blob>>>& files) {
    return false;
  }

  virtual bool SaveAssetFile(std::string_view asset_id, std::string_view filename,
                             std::variant<std::string, Blob> content) {
    return false;
  }

  virtual bool DeleteAsset(std::string_view asset_id) { return false; }
};

class DirectoryAssetLibrary : public AssetLibrary {
 public:
  DirectoryAssetLibrary(std::string_view directory);

  bool Contains(std::string_view asset_id) const override;
  std::vector<std::string> GetAssets() const override;
  std::string GetAssetType(std::string_view asset_id) const override;
  std::vector<std::string> GetAssetFileTypes(std::string_view asset_id) const override;
  std::optional<std::string> LoadAssetTextFile(std::string_view asset_id,
                                               std::string_view filename) const override;
  std::optional<Blob> LoadAssetBinaryFile(std::string_view asset_id, std::string_view filename) const override;
  std::vector<std::string> GetAssetsWithType(std::string_view type) const override;

  bool CreateAsset(std::string_view asset_id, std::string_view type,
                   const std::vector<std::pair<std::string, std::variant<std::string, Blob>>>& files) override;

  bool SaveAssetFile(std::string_view asset_id, std::string_view filename,
                     std::variant<std::string, Blob> content) override;

  bool DeleteAsset(std::string_view asset_id) override;

  void Rescan();
  inline std::string directory() const { return directory_; }

 protected:
  std::optional<std::string> GetAssetFilename(std::string_view asset_id, std::string_view filename) const;

 private:
  std::filesystem::path directory_;

  struct AssetData {
    std::filesystem::path directory;
    std::string type;
    std::vector<std::string> filenames;
  };

  std::unordered_map<std::string, AssetData> assets_;
  std::unordered_multimap<std::string, std::string> assets_with_type_;
};

namespace detail {
void SetEngineAssetLibrary(std::unique_ptr<AssetLibrary> asset_library);
void SetApplicationAssetLibrary(std::unique_ptr<AssetLibrary> asset_library);
}  // namespace detail

template <typename T, typename... Args>
void CreateEngineAssetLibrary(Args... args) {
  static_assert(std::is_base_of<AssetLibrary, T>::value, "T must derive from AssetLibrary");
  detail::SetEngineAssetLibrary(std::make_unique<T>(std::forward<Args>(args)...));
}
void SetEngineAssetsDirectory(std::string_view directory);

template <typename T, typename... Args>
void CreateApplicationAssetLibrary(Args... args) {
  static_assert(std::is_base_of<AssetLibrary, T>::value, "T must derive from AssetLibrary");
  detail::SetApplicationAssetLibrary(std::make_unique<T>(std::forward<Args>(args)...));
}
void SetApplicationAssetsDirectory(std::string_view directory);

AssetLibrary* GetEngineAssetLibrary();
AssetLibrary* GetApplicationAssetLibrary();

inline AssetLibrary* GetAssetLibraryForAsset(std::string_view asset_id) {
  return GetApplicationAssetLibrary()->Contains(asset_id) ? GetApplicationAssetLibrary() : GetEngineAssetLibrary();
}

}  // namespace ovis
