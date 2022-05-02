#pragma once

#include <catch2/catch.hpp>

#include <fmt/format.h>
#include <ovis/utils/result.hpp>

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
      UNSCOPED_INFO(fmt::format("{}", require_result.error().message)); \
    } \
    REQUIRE(require_result); \
  } while (false)
