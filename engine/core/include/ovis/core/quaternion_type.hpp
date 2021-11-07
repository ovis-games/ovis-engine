#pragma once

#include <cmath>
#include <type_traits>

#include <fmt/format.h>

#include <ovis/utils/json.hpp>
#include <ovis/core/math_constants.hpp>
#include <ovis/core/vector_types.hpp>

namespace ovis {

union Matrix3;

union alignas(sizeof(float) * 4) Quaternion {
  struct {
    float w;
    float x;
    float y;
    float z;
  };
  float data[4];

  static constexpr int ELEMENT_COUNT = 4;

  inline float& operator[](int index) { return data[index]; }
  inline float operator[](int index) const { return data[index]; }

  inline static constexpr Quaternion Identity();

  inline static Quaternion FromAxisAndAngle(const Vector3& axis, float angle);
  inline static Quaternion FromEulerAngles(float yaw, float pitch, float roll);
  // The matrix may noy contain any scaling (uniform scaling may be fine, TODO: check that)
  inline static Quaternion FromRotationMatrix(const Matrix3& rotation_matrix);
};
static_assert(sizeof(Quaternion) == 16);
static_assert(std::is_trivially_copyable<Quaternion>());
static_assert(std::is_standard_layout<Quaternion>());

template <typename T>
struct is_quaternion : public std::integral_constant<bool, std::is_same<T, Quaternion>::value> {};

}  // namespace ovis

template <>
struct fmt::formatter<ovis::Quaternion> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::Quaternion& quaternion, FormatContext& ctx) {
    return format_to(ctx.out(),
                     presentation == 'f' ? "({:.1f}, {:.1f}, {:.1f}, {:.1f})" : "({:.1e}, {:.1e}, {:.1e}, {:.1e})",
                     quaternion.w, quaternion.x, quaternion.y, quaternion.z);
  }
};

namespace nlohmann {

template <>
struct adl_serializer<ovis::Quaternion> {
  static void to_json(json& json, const ovis::Quaternion& quaternion) {
    json = {quaternion[0], quaternion[1], quaternion[2], quaternion[3]};
  }

  static void from_json(const json& json, ovis::Quaternion& quaternion) {
    for (int i = 0; i < 4; ++i) {
      json.at(i).get_to(quaternion[i]);
    }
  }
};

}  // namespace nlohmann
