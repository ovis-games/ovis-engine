#include <ovis/engine/game_settings.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/log.hpp>

namespace ovis {

// clang-format off
const json GameSettings::SCHEMA = {
  {"title", "Game Settings"},
  {"type", "object"},
  {"description", "Represents the general settings of a game."},
  {"properties", {
    {"Startup Scene", {
      {"type", "asset<scene>"},
      {"description", "The scene that is loaded when the game starts."},
    }}
  }}
};
// clang-format on

void to_json(json& data, const GameSettings& settings) {
  data = json{{
    "Startup Scene", settings.startup_scene
  }};
}

void from_json(const json& data, GameSettings& settings) {
  data.at("Startup Scene").get_to(settings.startup_scene);
}

std::optional<GameSettings> LoadGameSettings(const std::string& asset_id) {
  AssetLibrary* asset_library = GetApplicationAssetLibrary();
  if (!asset_library) {
    LogE("Asset library does not exist!");
    return {};
  }
  if (asset_library->GetAssetType(asset_id) != "settings") {
    LogE("Asset has invalid type: '{}' (should be 'settings')!", asset_library->GetAssetType(asset_id));
    return {};
  }
  std::optional<std::string> settings = asset_library->LoadAssetTextFile(asset_id, "json");
  if (!settings) {
    LogE("Could not load settings file");
    return {};
  }
  return json::parse(*settings);
}

}  // namespace ovis
