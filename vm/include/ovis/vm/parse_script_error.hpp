#pragma once

#include <vector>
#include <string>

#include <ovis/utils/result.hpp>

namespace ovis {

struct ParseScriptError : Error {
  ParseScriptError(std::string_view message) : Error(message) {}
  ParseScriptError(std::string_view message, std::string_view path) : Error(message), path(path) {}

  std::optional<std::string> path;
};
using ParseScriptErrors = std::vector<ParseScriptError>;

}  // namespace ovis
