#pragma once

#include <algorithm>
#include <type_traits>

#include <ovis/math/color_type.hpp>
#include <ovis/math/matrix_types.hpp>
#include <ovis/math/quaternion_type.hpp>
#include <ovis/math/vector_types.hpp>

namespace ovis {

template <typename T>
struct is_math_type : public std::integral_constant<bool, is_vector<T>::value || is_matrix<T>::value ||
                                                              is_quaternion<T>::value || is_color<T>::value> {};

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline bool operator==(const T& lhs, const T& rhs) {
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    if (!(lhs[i] == rhs[i])) {
      return false;
    }
  }
  return true;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline bool operator!=(const T& lhs, const T& rhs) {
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    if (lhs[i] != rhs[i]) {
      return true;
    }
  }
  return false;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T min(const T& value1, const T& value2) {
  using namespace std;

  T result;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = min(value1, value2);
  }
  return result;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T max(const T& value1, const T& value2) {
  using namespace std;

  T result;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = max(value1, value2);
  }
  return result;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T clamp(const T& value, const T& lo, const T& hi) {
  using namespace std;

  T result;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = clamp(value, lo, hi);
  }
  return result;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T operator+(const T& lhs, const T& rhs) {
  T result;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = lhs[i] + rhs[i];
  }
  return result;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T& operator+=(T& lhs, const T& rhs) {
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    lhs[i] += rhs[i];
  }
  return lhs;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T operator-(const T& rhs) {
  T result;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = -rhs[i];
  }
  return result;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T operator-(const T& lhs, const T& rhs) {
  T result;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = lhs[i] - rhs[i];
  }
  return result;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T& operator-=(T& lhs, const T& rhs) {
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    lhs[i] -= rhs[i];
  }
  return lhs;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T operator*(const T& lhs, float rhs) {
  T result;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = lhs[i] * rhs;
  }
  return result;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T operator*(float lhs, const T& rhs) {
  T result;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = lhs * rhs[i];
  }
  return result;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T& operator*=(T& lhs, float rhs) {
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    lhs[i] *= rhs;
  }
  return lhs;
}

template <typename T, std::enable_if_t<is_vector<T>::value || is_color<T>::value, bool> = true>
inline T operator*(const T& lhs, const T& rhs) {
  T result;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = lhs[i] * rhs[i];
  }
  return result;
}

template <typename T, std::enable_if_t<is_vector<T>::value || is_color<T>::value, bool> = true>
inline T operator*=(T& lhs, const T& rhs) {
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    lhs[i] *= rhs[i];
  }
  return lhs;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T operator/(const T& lhs, float rhs) {
  T result;
  const float inverse = 1.0f / rhs;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = lhs[i] * inverse;
  }
  return result;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T operator/(float lhs, const T& rhs) {
  T result;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = lhs / rhs[i];
  }
  return result;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T& operator/=(T& lhs, float rhs) {
  const float inverse = 1.0f / rhs;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    lhs[i] *= inverse;
  }
  return lhs;
}

template <typename T, std::enable_if_t<is_vector<T>::value || is_color<T>::value, bool> = true>
inline T operator/(const T& lhs, const T& rhs) {
  T result;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result[i] = lhs[i] / rhs[i];
  }
  return result;
}

template <typename T, std::enable_if_t<is_vector<T>::value || is_color<T>::value, bool> = true>
inline T operator/=(T& lhs, const T& rhs) {
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    lhs[i] /= rhs[i];
  }
  return lhs;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline float SquaredLength(const T& vector) {
  float result = 0.0f;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result += vector[i] * vector[i];
  }
  return result;
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline float Length(const T& vector) {
  return std::sqrt(SquaredLength(vector));
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline T Normalize(const T& vector) {
  return vector * (1.0f / Length(vector));
}

template <typename T, std::enable_if_t<is_math_type<T>::value, bool> = true>
inline float Dot(const T& lhs, const T& rhs) {
  float result = 0.0f;
  for (int i = 0; i < T::ELEMENT_COUNT; ++i) {
    result += lhs[i] * rhs[i];
  }
  return result;
}

}  // namespace ovis
