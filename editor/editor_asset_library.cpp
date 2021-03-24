#include "editor_asset_library.hpp"

#include "global.hpp"
#include <filesystem>
#include <fstream>

#include <fmt/format.h>
#include <microtar.h>

#include <ovis/utils/file.hpp>
#include <ovis/utils/log.hpp>
#include <ovis/application/application.hpp>
#include <ovis/networking/fetch.hpp>
#include <ovis/core/lua.hpp>

namespace ovis {
namespace editor {

EditorAssetLibrary::EditorAssetLibrary(const std::string& directory) : DirectoryAssetLibrary(directory) {
  if (!std::filesystem::exists(directory)) {
    if (!std::filesystem::create_directory(directory)) {
      LogE("Failed to create asset library directory '{}'", directory);
      SDL_assert(false);
    }
  } else if (!std::filesystem::is_directory(directory)) {
    LogE("'{}' is not a directory", directory);
    SDL_assert(false);
  }
}

bool EditorAssetLibrary::CreateAsset(
    const std::string& asset_id, const std::string& type,
    const std::vector<std::pair<std::string, std::variant<std::string, Blob>>>& files) {
  if (DirectoryAssetLibrary::CreateAsset(asset_id, type, files)) {
    for (const auto file : files) {
      UploadFile(*GetAssetFilename(asset_id, file.first));
    }
    return true;
  } else {
    return false;
  }
}

bool EditorAssetLibrary::SaveAssetFile(const std::string& asset_id, const std::string& file_type,
                                       std::variant<std::string, Blob> content) {
  if (DirectoryAssetLibrary::SaveAssetFile(asset_id, file_type, std::move(content))) {
    UploadFile(*GetAssetFilename(asset_id, file_type));
    return true;
  } else {
    return false;
  }
}

std::optional<Blob> EditorAssetLibrary::Package() {
  // Should not be necessary but better save than sorry
  Rescan();

  std::vector<std::byte> archive_data;

  mtar_t tar;
  tar.read = nullptr;
  tar.write = [](mtar_t* tar, const void* data, unsigned size) {
    auto archive_data = reinterpret_cast<std::vector<std::byte>*>(tar->stream);
    size_t current_size = archive_data->size();
    archive_data->resize(current_size + size);
    std::memcpy(archive_data->data() + current_size, data, size);
    return 0;
  };
  tar.seek = nullptr;
  tar.close = nullptr;
  tar.stream = &archive_data;
  tar.pos = 0;
  tar.remaining_data = 0;
  tar.last_header = 0;

  for (const std::string& asset_id : GetAssets()) {
    for (const std::string& asset_file_type : GetAssetFileTypes(asset_id)) {
      const std::optional<std::string> asset_filename = GetAssetFilename(asset_id, asset_file_type);
      if (!asset_filename) {
        LogE("Could not get asset filename for asset id '{}' and file type '{}'", asset_id, asset_file_type);
        return {};
      }

      LogV("Add asset file to archive ({},{})", asset_id, asset_file_type);
      // TODO: we should not reallocate this every time!
      std::optional<Blob> content = LoadAssetBinaryFile(asset_id, asset_file_type);
      if (!asset_filename) {
        LogE("Could load asset file for asset id '{}' and file type '{}'", asset_id, asset_file_type);
        return {};
      }

      if (int error = mtar_write_file_header(&tar, asset_filename->c_str(), content->size()); error != MTAR_ESUCCESS) {
        LogE("Failed to write file header to archive: {}", mtar_strerror(error));
        return {};
      }

      if (int error = mtar_write_data(&tar, content->data(), content->size()); error != MTAR_ESUCCESS) {
        LogE("Failed to write file to archive: {}", mtar_strerror(error));
        return {};
      }
    }
  }

  if (int error = mtar_finalize(&tar); error != MTAR_ESUCCESS) {
    LogE("Failed to finalize archive: {}", mtar_strerror(error));
    return {};
  }

  return archive_data;
}

// void AssetLibrary::AddScene(const std::string& id, const std::string& path) {
//   SDL_assert(!AssetExists(id));
//   if (EnsurePath(path)) {
//     const std::string scene_filename = GetAssetFilename(path, id, "scene.json");
//     SDL_assert(!std::filesystem::exists(scene_filename));
//     std::ofstream scene_file(scene_filename);
//     const json scene_data = {{"version", "0.1"}, {"objects", json::object()}};
//     scene_file << scene_data;
//     scene_file.close();
//     LogV("Created file '{}'", scene_filename);

//     assets_data_["assets"][id] = {{"type", "scene"}, {"path", path}};
//     SaveAssetsFile();
//     UploadFile(scene_filename);
//   }
// }

// void AssetLibrary::AddSceneControllerScript(const std::string& id, const std::string& path) {
//   SDL_assert(!AssetExists(id));
//   if (EnsurePath(path)) {
//     const std::string script_options_filename = GetAssetFilename(path, id, "script.json");
//     SDL_assert(!std::filesystem::exists(script_options_filename));
//     std::ofstream script_options_file(script_options_filename);
//     const json script_options = {{"version", "0.1"}, {"type", "scene_controller"}};
//     script_options_file << script_options;
//     script_options_file.close();
//     LogV("Created file '{}'", script_options_filename);

//     const std::string lua_filename = GetAssetFilename(path, id, "script.lua");
//     SDL_assert(!std::filesystem::exists(lua_filename));
//     std::ofstream lua_file(lua_filename);
//     const std::string source = fmt::format(
//         R"({0} = class('{0}')

// function {0}:Update(delta_time)
//   LogI('Hello from {0}')
// end)",
//         id);
//     lua_file << source;
//     lua_file.close();
//     LogV("Created file '{}'", lua_filename);

//     assets_data_["assets"][id] = {{"type", "scene_controller"}, {"path", path}};
//     SaveAssetsFile();

//     UploadFile(script_options_filename);
//     UploadFile(lua_filename);
//     Lua::LoadSceneControllerScript(id, "/assets/" + id + ".script.lua");
//   }
// }

void EditorAssetLibrary::UploadFile(const std::string& filename) {
  // std::filesystem::relative(filename, directory_);
  // TODO: check if inside directory_
  files_to_upload_.insert(filename);
  UploadNextFile();
}

void EditorAssetLibrary::UploadNextFile() {
  if (!is_currently_uploading_ && files_to_upload_.size() > 0) {
    const std::string filename = *files_to_upload_.begin();
    files_to_upload_.erase(filename);
    is_currently_uploading_ = true;

    auto file_data = LoadBinaryFile(filename);
    if (file_data) {
      LogI("Uploading {} ({} bytes)", filename, file_data->size());

      std::string relative_filename = std::filesystem::relative(filename, directory());
      std::string url =
          fmt::format("{}/v1/games/{}/assetFiles/{}", backend_url, project_id, relative_filename);
      FetchOptions options;
      options.method = RequestMethod::PUT;
      options.headers["Content-Type"] = "application/octet-stream";
      options.on_success = [this, filename](const FetchResponse&) {
        LogI("Successfully uploaded '{}'", filename);
        is_currently_uploading_ = false;
        UploadNextFile();
      };
      options.on_error = [this, filename](const FetchResponse&) {
        LogE("Failed to upload '{}'", filename);
        is_currently_uploading_ = false;
        UploadNextFile();
      };
      Fetch(url, options, std::move(*file_data));
    } else {
      LogE("Failed to upload '{}' file does not exist", filename);
    }
  }
}

}  // namespace editor
}  // namespace ovis
