#pragma once

#include <optional>
#include <ovis/core/json.hpp>

namespace ovis
{

struct GameSettings {
  std::string startup_scene;

  static const json SCHEMA;
};

void to_json(json& data, const GameSettings& settings);
void from_json(const json& data, GameSettings& settings);

std::optional<GameSettings> LoadGameSettings(const std::string& asset_id = "GameSettings");

} // namespace ovis
