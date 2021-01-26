#include <ovis/engine/game_settings.hpp>

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

}  // namespace ovis
