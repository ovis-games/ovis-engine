#pragma once

#include <catch2/catch.hpp>

#include <fmt/format.h>
#include <ovis/utils/result.hpp>
#include <ovis/vm/parse_script_error.hpp>

// #define REQUIRE_RESULT(expr) \
//   do { \
//     auto&& require_result = expr; \
//     if (!require_result) { \
//       for (const auto& error : require_result.error()) { \
//         UNSCOPED_INFO(fmt::format("{}: {}", error.path.value_or(""), error.message)); \
//       } \
//     } \
//     REQUIRE(require_result); \
//   } while (false)

template<>
struct fmt::formatter<ovis::Error> {
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(const ovis::Error& error, FormatContext& ctx) {
    return fmt::format_to(ctx.out(), "{}", error.message);
  }
};

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

#define REQUIRE_RESULT(expr) \
  do { \
    auto&& require_result = expr; \
    if (!require_result) { \
      UNSCOPED_INFO(fmt::format("{}", require_result.error())); \
    } \
    REQUIRE(require_result); \
  } while (false)

// #define REQUIRE_NO_ERRORS(expr) \
//   do { \
//     auto&& require_result = expr; \
//     if (!require_result) { \
//       for (const auto& error : require_result.error()) { \
//         UNSCOPED_INFO(fmt::format("{}: {}", error.path.value_or(""), error.message)); \
//       } \
//     } \
//     REQUIRE(require_result); \
//   } while (false)
