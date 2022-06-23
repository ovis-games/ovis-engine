#pragma once

#include <string>
#include <vector>

#include "ovis/utils/result.hpp"

namespace ovis {

struct ScriptErrorLocation {
  template <typename... FormatArgs>
  ScriptErrorLocation(std::string_view script_name, std::string_view path, FormatArgs&&... format_arguments)
      : script_name(script_name), json_path(fmt::format(path, std::forward<FormatArgs>(format_arguments)...)) {}

  std::string script_name;
  std::string json_path;
};

struct ParseScriptError : Error {
  template <typename... Args>
  ParseScriptError(std::optional<ScriptErrorLocation> location, std::string_view message, Args&&... args)
      : Error(message, std::forward<Args>(args)...), location(location) {}

  std::optional<ScriptErrorLocation> location;
};
using ParseScriptErrors = std::vector<ParseScriptError>;

}  // namespace ovis
