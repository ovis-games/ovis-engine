#include "asset_importer.hpp"

#include <filesystem>

#include <stb_image.h>

#include <ovis/utils/json.hpp>
#include <ovis/utils/utf8.hpp>
#include <ovis/core/asset_library.hpp>

namespace ovis {
namespace editor {

void ImportImage(const std::string& filename) {
  LogV("Importing image: {}", filename);

  int width;
  int height;
  int channels;
  stbi_uc* data = stbi_load(filename.c_str(), &width, &height, &channels, STBI_default);

  if (data != nullptr) {
    static const char* const FORMATS[] = {"", "R_UINT8", "RG_UINT8", "RGB_UINT8", "RGBA_UINT8"};

    json parameters;
    parameters["width"] = width;
    parameters["height"] = height;
    parameters["filter"] = "bilinear";
    parameters["format"] = FORMATS[channels];  // TODO: dangerous, pls change

    std::vector<std::byte> blob(channels * width * height);
    std::memcpy(blob.data(), data, channels * width * height);

    if (GetApplicationAssetLibrary()->CreateAsset(
            std::filesystem::path(filename).stem().string(),  // TODO: do '.' have to be replaces with e.g., '_'?
            "texture2d", {{"json", parameters.dump()}, {"0", blob}})) {
      LogI("Successfully imported {}", std::filesystem::path(filename).filename().string());
    }
  } else {
    LogE("Failed to load image: {}", stbi_failure_reason());
  }

  stbi_image_free(data);
}

void ImportAsset(const std::string& filename) {
  const std::string extension = to_lower(std::filesystem::path(filename).extension());

  if (extension == ".jpeg" || extension == ".jpg" || extension == ".jps" || extension == ".png" ||
      extension == ".tga" || extension == ".bmp" || extension == ".psd" || extension == ".gif" || extension == ".hdr" ||
      extension == ".pic" || extension == ".pnm") {
    ImportImage(filename);
  } else {
    LogE("Unknown file format: {}", extension);
  }
}

}  // namespace editor
}  // namespace ovis
