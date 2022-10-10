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
