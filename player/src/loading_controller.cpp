#if OVIS_EMSCRIPTEN

#include <filesystem>

#include <imgui.h>
#include <ovis/player/loading_controller.hpp>

#include <ovis/utils/file.hpp>
#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/networking/fetch.hpp>
#include <ovis/application/game_settings.hpp>

namespace ovis {
namespace player {

LoadingController::LoadingController(std::string_view backend_prefix, std::string_view user_name,
                                     std::string_view game_name, std::string_view package_type)
    : SceneController("LoadingController") {
  if (!std::filesystem::exists("/assets")) {
    std::filesystem::create_directory("/assets");
  }
  if (!std::filesystem::exists("/tmp")) {
    std::filesystem::create_directory("/tmp");
  }

  const std::string url =
      fmt::format("{}/v1/games/{}/{}/packages/{}", backend_prefix, user_name, game_name, package_type);

  FetchOptions options;
  options.method = RequestMethod::GET;
  options.on_success = [this](const FetchResponse& response) {
    package_.resize(response.content_length);
    std::memcpy(package_.data(), response.body, response.content_length);

    tar_.read = [](mtar_t* tar, void* data, unsigned size) -> int {
      LogI("Read {} bytes (from {})", size, tar->pos);

      auto package_data = reinterpret_cast<std::vector<std::byte>*>(tar->stream);

      if (tar->pos + size > package_data->size()) {
        return MTAR_EREADFAIL;
      }

      std::memcpy(data, package_data->data() + tar->pos, size);

      return MTAR_ESUCCESS;
    };
    tar_.write = nullptr;
    tar_.seek = [](mtar_t* tar, unsigned pos) -> int {
      LogI("Seek to: {}", pos);
      return MTAR_ESUCCESS;
    };
    tar_.close = nullptr;
    tar_.stream = &package_;
    tar_.pos = 0;
    tar_.remaining_data = 0;
    tar_.last_header = 0;

    state_ = State::EXTRACTING;
  };
  options.on_error = [this](const FetchResponse&) { LogE("Failed to download package!"); };
  options.on_progress = [this](const FetchProgress& progress) {
    LogI("{}/{}", progress.num_bytes, progress.total_bytes);
  };

  Fetch(url, options);
}

void LoadingController::Update(std::chrono::microseconds ms) {
  if (state_ == State::EXTRACTING) {
    LogI("Read next header!");

    mtar_header_t header;
    if ((mtar_read_header(&tar_, &header)) != MTAR_ENULLRECORD) {
      LogI("File {} ({} bytes, type={})", header.name, header.size, header.type);

      file_buffer_.resize(header.size);
      mtar_read_data(&tar_, file_buffer_.data(), header.size);
      WriteBinaryFile(header.name, file_buffer_);

      LogI("Next file!");
      mtar_next(&tar_);
      progress_ = static_cast<float>(tar_.pos) / package_.size();
    } else {
      // We are done
      SetApplicationAssetsDirectory("/assets");
      const std::optional<GameSettings> settings = LoadGameSettings();

      // TODO: handle errors here appropriately
      if (!settings) {
        LogE("Could not load game settings!");
        state_ = State::ERROR;
        return;
      }

      AssetLibrary* asset_library = GetApplicationAssetLibrary();
      if (!asset_library) {
        LogE("Asset library does not exist!");
        state_ = State::ERROR;
        return;
      }
      if (const auto asset_type = asset_library->GetAssetType(settings->startup_scene); !asset_type || *asset_type != "scene") {
        LogE("Asset has invalid type: '{}' (should be 'scene')!", asset_library->GetAssetType(settings->startup_scene));
        state_ = State::ERROR;
        return;
      }
      Result<std::string> serialized_scene = asset_library->LoadAssetTextFile(settings->startup_scene, "json");
      if (!serialized_scene) {
        LogE("Could not load settings file");
        state_ = State::ERROR;
        return;
      }

      scene()->Deserialize(json::parse(*serialized_scene));
      // Remove(); (not needed, the deserialize above will remove the controller automatically)
    }
  }
}

}  // namespace player

}  // namespace ovis

#endif
