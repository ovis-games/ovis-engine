#pragma once

#include <cstddef>
#include <tuple>

namespace ovis {

template <std::size_t N, typename... Types>
using nth_parameter_t = std::tuple_element_t<N, std::tuple<Types...>>;
static_assert(std::is_same_v<void, nth_parameter_t<0, void, bool, int>>);
static_assert(std::is_same_v<bool, nth_parameter_t<1, void, bool, int>>);
static_assert(std::is_same_v<int, nth_parameter_t<2, void, bool, int>>);

}
