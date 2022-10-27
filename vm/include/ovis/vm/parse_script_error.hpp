#pragma once

#include <string>
#include <vector>

#include <fmt/format.h>

#include "ovis/utils/result.hpp"

namespace ovis {

struct ScriptErrorLocation {
  template <typename... FormatArgs>
  ScriptErrorLocation(std::string_view script_name, std::string_view path)
      : script_name(script_name), json_path(path) {}

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

template<>
struct fmt::formatter<ovis::ParseScriptErrors> {
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(const ovis::ParseScriptErrors& errors, FormatContext& ctx) {
    for (const auto& error : errors) {
      if (error.location.has_value()) {
        fmt::format_to(ctx.out(), "{}:{} {}\n", error.location->script_name, error.location->json_path, error.message);
      } else {
        fmt::format_to(ctx.out(), "{}", error.message);
      }
    }
    return fmt::format_to(ctx.out(), "");
  }
};

