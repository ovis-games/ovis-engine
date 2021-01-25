#pragma once

#include <variant>

namespace ovis {

template <typename T, typename VariantType>
inline T get_with_default(const VariantType& variant, const T& default_value) {
  if (const T* value = std::get_if<T>(&variant); value != nullptr) {
    return *value;
  } else {
    return default_value;
  }
}

}  // namespace ovis
