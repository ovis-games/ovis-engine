#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <ovis/utils/result.hpp>

namespace ovis {

using Blob = std::vector<std::byte>;

// The 'filename' parameters remain std::string references instead of std::string_view as the fstream constructor
// requires a null terminated string. So if the function caller already has an std::string converting it to a string
// view and reconstructing a string inside the function is a waste.
// TODO: maybe an overload with a const char* should be offered.

std::string ExtractDirectory(const std::string& file_path);
Result<std::string> LoadTextFile(const std::string& filename);
Result<Blob> LoadBinaryFile(const std::string& filename);

Result<> WriteTextFile(const std::string& filename, std::string_view content);
Result<> WriteBinaryFile(const std::string& filename, const Blob& content);

}  // namespace ovis
