#include <fstream>
#include <vector>

#include <ovis/utils/file.hpp>

namespace ovis {

std::string ExtractDirectory(const std::string& file_path) {
  return file_path.substr(0, file_path.find_last_of('/'));
}

Result<std::string> LoadTextFile(const std::string& filename) {
  std::ifstream file(filename);

  if (!file) {
    return Error("Cannot open file: {}", filename);
  }

  file.seekg(0, std::ios::end);
  std::string string(file.tellg(), '\0');
  file.seekg(0, std::ios::beg);
  if (!file.read(string.data(), string.size())) {
    return Error("Could not read file: {}", filename);
  }

  return std::move(string);
}

Result<Blob> LoadBinaryFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);

  if (!file) {
    return Error("Cannot open file: {}", filename);
  }

  file.seekg(0, std::ios::end);
  Blob buffer(file.tellg());
  file.seekg(0, std::ios::beg);
  if (!file.read(reinterpret_cast<char*>(buffer.data()), buffer.size())) {
    return Error("Could not read file: {}", filename);
  }

  return buffer;
}

Result<> WriteTextFile(const std::string& filename, std::string_view content) {
  std::ofstream file(filename);

  if (!file) {
    return Error("Cannot open file: {}", filename);
  }

  if (!file.write(content.data(), content.size())) {
    return Error("Could not write file: {}", filename);
  }

  return {};
}

Result<> WriteBinaryFile(const std::string& filename, const Blob& content) {
  std::ofstream file(filename, std::ios::binary | std::ios::trunc);

  if (!file) {
    return Error("Cannot open file: {}", filename);
  }

  if (!file.write(reinterpret_cast<const char*>(content.data()), content.size())) {
    return Error("Could not write file: {}", filename);
  }

  return {};
}

}  // namespace ovis
