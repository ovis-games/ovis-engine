#pragma once

#include <emscripten/val.h>

#include "ovis/core/asset_library.hpp"

namespace ovis {

class AssetProvider : public AssetLibrary {
 public:
  AssetProvider(const emscripten::val& asset_provider);

  bool Contains(std::string_view asset_id) const override final;
  std::vector<std::string> GetAssets() const override final;
  Result<std::string> GetAssetType(std::string_view asset_id) const override final;
  std::vector<std::string> GetAssetFileTypes(std::string_view asset_id) const override final;
  std::vector<std::string> GetAssetsWithType(std::string_view type) const override final;

  Result<std::string> LoadAssetTextFile(std::string_view asset_id,
                                               std::string_view filename) const override final;
  Result<Blob> LoadAssetBinaryFile(std::string_view asset_id, std::string_view filename) const override final;

 private:
  emscripten::val asset_provider_;
};

} 
