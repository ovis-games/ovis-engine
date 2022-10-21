#pragma once

#include "catch2/catch.hpp"
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

// Define the namespace to make the statement 'using namespace ovis' not an error in snippets.
namespace ovis {}
