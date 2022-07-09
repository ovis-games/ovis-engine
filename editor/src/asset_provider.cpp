#include "ovis/editor/asset_provider.hpp"

#include <emscripten/bind.h>

namespace ovis {

namespace {

std::vector<std::string> GetObjectKeys(const emscripten::val& object) {
  emscripten::val global_object = emscripten::val::global("Object");
  emscripten::val keys = global_object.call<emscripten::val>("keys", object);
  return emscripten::vecFromJSArray<std::string>(keys);
}

void setAssetProvider(const emscripten::val& asset_provider) {
  ovis::CreateApplicationAssetLibrary<AssetProvider>(asset_provider);
}

EMSCRIPTEN_BINDINGS(editor_module) {
  function("setAssetProvider", setAssetProvider);
}

}  // namespace

AssetProvider::AssetProvider(const emscripten::val& asset_provider) : asset_provider_(asset_provider) {
  LogD("Assets:");
  for (const auto& asset_id : GetAssets()) {
    LogD(asset_id);
  }
}

bool AssetProvider::Contains(std::string_view asset_id) const {
  const auto assets = GetAssets();
  return std::find_if(assets.begin(), assets.end(), [asset_id](const std::string& id) { return id == asset_id; }) !=
         assets.end();
}

std::vector<std::string> AssetProvider::GetAssets() const {
  return GetObjectKeys(asset_provider_["assets"]);
}

Result<std::string> AssetProvider::GetAssetType(std::string_view asset_id) const {
  if (!Contains(asset_id)) {
    return Error("Asset {} does not exist", asset_id);
  }

  const std::string asset_id_string(asset_id);
  return asset_provider_["assets"][asset_id_string]["type"].as<std::string>();
}

std::vector<std::string> AssetProvider::GetAssetFileTypes(std::string_view asset_id) const {
  if (!Contains(asset_id)) {
    return {};
  }

  const std::string asset_id_string(asset_id);
  return GetObjectKeys(asset_provider_["assets"][asset_id_string]["files"]);
}

Result<std::string> AssetProvider::LoadAssetTextFile(std::string_view asset_id, std::string_view filename) const {
  LogD("LoadAssetTextFile {} {}", asset_id, filename);
  const std::string asset_id_string(asset_id);
  const std::string filename_string(filename);
  emscripten::val content = asset_provider_["assets"][asset_id_string]["files"][filename_string]["content"];
  if (const auto content_type = content.typeOf().as<std::string>(); content_type != "object") {
    return Error("Invalid asset content: {}", content_type);
  } else {
    return emscripten::val::global("JSON").call<std::string>("stringify", content);
  }
}

Result<Blob> AssetProvider::LoadAssetBinaryFile(std::string_view asset_id, std::string_view filename) const {
  LogD("LoadAssetBinaryFile {} {}", asset_id, filename);
  return Error("Not implemented");
}

std::vector<std::string> AssetProvider::GetAssetsWithType(std::string_view type) const {
  LogD("GetAssetsWithType {}", type);
  return {};
}

}  // namespace ovis

