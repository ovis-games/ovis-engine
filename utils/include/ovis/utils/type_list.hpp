#pragma once

#include <cstdint>

namespace ovis {

template <typename... Ts> struct TypeList {
  static constexpr std::size_t size = sizeof...(Ts);
};

}  // namespace ovis
