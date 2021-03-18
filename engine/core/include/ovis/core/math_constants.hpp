#pragma once

#include <cmath>
#include <limits>
#include <type_traits>

namespace ovis {

template <typename T>
inline constexpr T Infinity() {
  return std::numeric_limits<T>::infinity();
}

template <typename T>
inline constexpr T NotANumber() {
  return std::numeric_limits<T>::quiet_NaN();
}

template <typename T>
inline constexpr T Pi() {
  return T{3.14159265358979323846};
}

template <typename T>
inline constexpr T TwoPi() {
  return T{2} * Pi<T>();
}

template <typename T>
inline constexpr T PiOverTwo() {
  return Pi<T>() / T{2};
}

template <typename T>
inline constexpr T PiOverFour() {
  return Pi<T>() / T{4};
}

template <typename T>
inline constexpr T OneOverPi() {
  return T{1} / Pi<T>();
}

template <typename T>
inline constexpr T DegreesToRadiansFactor() {
  return Pi<T>() / T{180};
}

template <typename T>
inline constexpr T RadiansToDegreesFactor() {
  return T{180} / Pi<T>();
}

}  // namespace ovis
