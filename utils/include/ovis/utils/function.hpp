#pragma once

namespace ovis {

template <typename Signature, typename C>
constexpr auto SelectOverload(Signature C::*member_function_pointer) {
  return member_function_pointer;
}

}  // namespace ovis
