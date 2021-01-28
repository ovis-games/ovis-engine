#pragma once

#include <memory>
#include <nlohmann/json.hh>

namespace ovis {

using json = nlohmann::json;

class AssetLibrary;
std::shared_ptr<json> LoadJsonSchema(AssetLibrary* asset_library, const std::string& name, bool use_cache = true);
std::shared_ptr<json> LoadJsonSchema(const std::string& name, bool use_cache = true);

}  // namespace ovis
