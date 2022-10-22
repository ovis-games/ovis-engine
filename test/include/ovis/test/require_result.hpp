#pragma once

#include "catch2/catch_test_macros.hpp"
#include "fmt/format.h"

#include "ovis/utils/result.hpp"

#define REQUIRE_RESULT(expr) \
  do { \
    auto&& require_result = expr; \
    if (!require_result) { \
      UNSCOPED_INFO(fmt::format("{}", require_result.error())); \
    } \
    REQUIRE(require_result); \
  } while (false)
