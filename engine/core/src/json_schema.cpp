#include <optional>
#include <unordered_map>

#include <ovis/utils/json.hpp>
#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>

namespace ovis {

namespace {

std::unordered_map<std::string, std::shared_ptr<json>> cached_schemas;

}  // namespace

std::shared_ptr<json> LoadJsonSchema(AssetLibrary* asset_library, const std::string& name, bool use_cache) {
  if (use_cache) {
    auto schema_iterator = cached_schemas.find(name);
    if (schema_iterator != cached_schemas.end()) {
      return schema_iterator->second;
    }
  }

  if (!asset_library->Contains(name) || asset_library->GetAssetType(name) != "schema") {
    LogE("Cannot load schema: {}", name);
    return nullptr;
  }

  std::optional<std::string> serialized_schema = asset_library->LoadAssetTextFile(name, "json");
  if (!serialized_schema) {
    LogE("Cannot load schema: {}", name);
    return nullptr;
  }

  return cached_schemas[name] = std::make_shared<json>(json::parse(*serialized_schema));
}

std::shared_ptr<json> LoadJsonSchema(const std::string& name, bool use_cache) {
  return LoadJsonSchema(
      GetApplicationAssetLibrary()->Contains(name) ? GetApplicationAssetLibrary() : GetEngineAssetLibrary(), name,
      use_cache);
}

}  // namespace ovis
