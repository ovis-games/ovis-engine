#include "editor_asset_library.hpp"

#include <filesystem>
#include <fstream>

#include <emscripten/fetch.h>
#include <fmt/format.h>

#include <ovis/core/file.hpp>
#include <ovis/core/log.hpp>

#include <ovis/engine/engine.hpp>
#include <ovis/engine/lua.hpp>

namespace ove {

EditorAssetLibrary::EditorAssetLibrary(const std::string& directory) : ovis::DirectoryAssetLibrary(directory) {
  if (!std::filesystem::exists(directory)) {
    if (!std::filesystem::create_directory(directory)) {
      ovis::LogE("Failed to create asset library directory '{}'", directory);
      SDL_assert(false);
    }
  } else if (!std::filesystem::is_directory(directory)) {
    ovis::LogE("'{}' is not a directory", directory);
    SDL_assert(false);
  }
}

bool EditorAssetLibrary::SaveAssetFile(const std::string& asset_id, const std::string& file_type,
                                       std::variant<std::string, ovis::Blob> content) {
  if (DirectoryAssetLibrary::SaveAssetFile(asset_id, file_type, std::move(content))) {
    UploadFile(*GetAssetFilename(asset_id, file_type));
    return true;
  } else {
    return false;
  }
}

// void AssetLibrary::AddScene(const std::string& id, const std::string& path) {
//   SDL_assert(!AssetExists(id));
//   if (EnsurePath(path)) {
//     const std::string scene_filename = GetAssetFilename(path, id, "scene.json");
//     SDL_assert(!std::filesystem::exists(scene_filename));
//     std::ofstream scene_file(scene_filename);
//     const ovis::json scene_data = {{"version", "0.1"}, {"objects", ovis::json::object()}};
//     scene_file << scene_data;
//     scene_file.close();
//     ovis::LogV("Created file '{}'", scene_filename);

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
//     const ovis::json script_options = {{"version", "0.1"}, {"type", "scene_controller"}};
//     script_options_file << script_options;
//     script_options_file.close();
//     ovis::LogV("Created file '{}'", script_options_filename);

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
//     ovis::LogV("Created file '{}'", lua_filename);

//     assets_data_["assets"][id] = {{"type", "scene_controller"}, {"path", path}};
//     SaveAssetsFile();

//     UploadFile(script_options_filename);
//     UploadFile(lua_filename);
//     ovis::Lua::LoadSceneControllerScript(id, "/assets/" + id + ".script.lua");
//   }
// }

void EditorAssetLibrary::UploadFile(const std::string& filename) {
  // std::filesystem::relative(filename, directory_);
  // TODO: check if inside directory_
  files_to_upload_.insert(filename);
  UploadNextFile();
}

void EditorAssetLibrary::UploadNextFile() {
  static const char* REQUEST_HEADERS[] = {"Content-Type", "application/octet-stream", 0};

  if (!is_currently_uploading_ && files_to_upload_.size() > 0) {
    const std::string filename = *files_to_upload_.begin();
    files_to_upload_.erase(filename);
    is_currently_uploading_ = true;

    auto file_data = ovis::LoadBinaryFile(filename);

    if (file_data) {
      current_file_content_ = std::move(*file_data);

      emscripten_fetch_attr_t attr;
      emscripten_fetch_attr_init(&attr);
      strcpy(attr.requestMethod, "PUT");
      attr.onsuccess = &EditorAssetLibrary::UploadSucceeded;
      attr.onerror = &EditorAssetLibrary::UploadFailed;
      attr.requestData = reinterpret_cast<const char*>(current_file_content_.data());
      attr.requestDataSize = current_file_content_.size();
      attr.requestHeaders = REQUEST_HEADERS;
      attr.userData = this;

      std::string relative_filename = std::filesystem::relative(filename, directory());
      std::string url = fmt::format("http://localhost:3000/v0/project/x/file/{}", relative_filename);
      emscripten_fetch(&attr, url.c_str());

      ovis::LogI("Uploading {} ({} bytes)", filename, file_data->size());
    } else {
      ovis::LogE("Failed to upload '{}' file does not exist", filename);
    }
  }
}

void EditorAssetLibrary::UploadSucceeded(emscripten_fetch_t* fetch) {
  EditorAssetLibrary* asset_library = reinterpret_cast<EditorAssetLibrary*>(fetch->userData);
  ovis::LogI("Successfully uploaded '{}', status code: {}", fetch->url, fetch->status);
  emscripten_fetch_close(fetch);
  asset_library->is_currently_uploading_ = false;

  asset_library->UploadNextFile();
}

void EditorAssetLibrary::UploadFailed(emscripten_fetch_t* fetch) {
  EditorAssetLibrary* asset_library = reinterpret_cast<EditorAssetLibrary*>(fetch->userData);
  ovis::LogE("Failed to upload '{}', status code: {}", fetch->url, fetch->status);
  emscripten_fetch_close(fetch);
  asset_library->is_currently_uploading_ = false;

  asset_library->UploadNextFile();
}

}  // namespace ove
