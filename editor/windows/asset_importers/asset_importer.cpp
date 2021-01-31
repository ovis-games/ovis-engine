#include "asset_importer.hpp"

#include <filesystem>

#include <stb_image.h>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/json.hpp>
#include <ovis/core/utf8.hpp>

namespace ove {

void ImportImage(const std::string& filename) {
  ovis::LogV("Importing image: {}", filename);

  int width;
  int height;
  int channels;
  stbi_uc* data = stbi_load(filename.c_str(), &width, &height, &channels, STBI_default);

  if (data != nullptr) {
    static const char* const FORMATS[] = {"", "R_UINT8", "RG_UINT8", "RGB_UINT8", "RGBA_UINT8"};

    ovis::json parameters;
    parameters["width"] = width;
    parameters["height"] = height;
    parameters["filter"] = "bilinear";
    parameters["format"] = FORMATS[channels];  // TODO: dangerous, pls change

    std::vector<std::byte> blob(channels * width * height);
    std::memcpy(blob.data(), data, channels * width * height);

    if (ovis::GetApplicationAssetLibrary()->CreateAsset(
            std::filesystem::path(filename).stem(),  // TODO: do '.' have to be replaces with e.g., '_'?
            "texture2d", {{"json", parameters.dump()}, {"0", blob}})) {
      ovis::LogI("Successfully imported {}", std::filesystem::path(filename).filename().string());
    }
  } else {
    ovis::LogE("Failed to load image: {}", stbi_failure_reason());
  }

  stbi_image_free(data);
}

void ImportAsset(const std::string& filename) {
  const std::string extension = ovis::to_lower(std::filesystem::path(filename).extension());

  if (extension == ".jpeg" || extension == ".jpg" || extension == ".jps" || extension == ".png" ||
      extension == ".tga" || extension == ".bmp" || extension == ".psd" || extension == ".gif" || extension == ".hdr" ||
      extension == ".pic" || extension == ".pnm") {
    ImportImage(filename);
  } else {
    ovis::LogE("Unknown file format: {}", extension);
  }
}

}  // namespace ove