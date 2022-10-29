#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "fmt/core.h"

#include "ovis/core/rect.hpp"

namespace ovis {

struct Display {
  std::string name;
  float diagonal_dpi;
  float vertical_dpi;
  float horizontal_dpi;
  Rect<int> bounds;
};

std::vector<Display> GetDisplays();

}  // namespace ovis

template <>
class fmt::formatter<ovis::Display> {
 public:
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
  template <typename Context>
  constexpr auto format(const ovis::Display& display, Context& ctx) const {
    return fmt::format_to(
      ctx.out(), 
      "{} {}x{} {}({}/{})DPI",
      display.name,
      display.bounds.width, display.bounds.height,
      display.diagonal_dpi, display.horizontal_dpi, display.vertical_dpi);
  }
};
