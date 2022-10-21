#pragma once

#define REQUIRE_RESULT(expr) \
  do { \
    auto&& require_result = expr; \
    if (!require_result) { \
      UNSCOPED_INFO(fmt::format("{}", require_result.error())); \
    } \
    REQUIRE(require_result); \
  } while (false)
