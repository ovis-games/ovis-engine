#pragma once

#include <vector>
#include <string>

#include <ovis/utils/result.hpp>

namespace ovis {

struct ParseScriptError : Error {
  template <typename... Args>
  ParseScriptError(std::optional<std::string_view> path, std::string_view message, Args&&... args)
      : Error(message, std::forward<Args>(args)...), path(path ? std::optional<std::string>(std::string(*path)) : std::nullopt) {}

  std::optional<std::string> path;
};
using ParseScriptErrors = std::vector<ParseScriptError>;

}  // namespace ovis
