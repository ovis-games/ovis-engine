#pragma once

#include <cmath>
#include <type_traits>

#include <fmt/format.h>

#include "ovis/utils/json.hpp"
#include "ovis/core/math_constants.hpp"
#include "ovis/core/quaternion_type.hpp"
#include "ovis/core/vector_types.hpp"

namespace ovis {

template <int ROW_COUNT, int COLUMN_COUNT>
struct MatrixTypes;

union Matrix2 {
  Vector2 rows[2];
  float data[4];

  inline Vector2& operator[](int row_index) { return rows[row_index]; }
  inline Vector2 operator[](int row_index) const { return rows[row_index]; }

  static constexpr int ELEMENT_COUNT = 2;
  static constexpr int ROW_COUNT = 2;
  static constexpr int COLUMN_COUNT = 2;

  inline static Matrix2 FromVector(const Vector4& vector);
};
static_assert(sizeof(Matrix2) == 16);
static_assert(std::is_trivially_copyable<Matrix2>());
static_assert(std::is_standard_layout<Matrix2>());
template <>
struct MatrixTypes<2, 2> {
  using Type = Matrix2;
};

union Matrix3 {
  Vector3 rows[3];

  inline constexpr Vector3& operator[](int row_index) { return rows[row_index]; }
  inline constexpr Vector3 operator[](int row_index) const { return rows[row_index]; }

  static constexpr int ELEMENT_COUNT = 3;
  static constexpr int ROW_COUNT = 3;
  static constexpr int COLUMN_COUNT = 3;
};
static_assert(sizeof(Matrix3) == 48);
static_assert(std::is_trivially_copyable<Matrix3>());
static_assert(std::is_standard_layout<Matrix3>());
template <>
struct MatrixTypes<3, 3> {
  using Type = Matrix3;
};

union Matrix3x4 {
  Vector4 rows[3];
  float data[12];

  inline constexpr Vector4& operator[](int row_index) { return rows[row_index]; }
  inline constexpr Vector4 operator[](int row_index) const { return rows[row_index]; }

  static constexpr int ELEMENT_COUNT = 3;
  static constexpr int ROW_COUNT = 3;
  static constexpr int COLUMN_COUNT = 4;

  inline static constexpr Matrix3x4 IdentityTransformation() {
    return {{
        // clang-format off
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        // clang-format on
    }};
  }

  inline static constexpr Matrix3x4 FromTransformation(const Vector3& translation, const Vector3& scaling,
                                                       const Quaternion& rotation);
  inline static constexpr Matrix3x4 FromTranslation(const Vector3& translation);
  inline static constexpr Matrix3x4 FromScaling(const Vector3& scaling);
  inline static constexpr Matrix3x4 FromScaling(float scaling);
  inline static constexpr Matrix3x4 FromRotation(const Quaternion& rotation);
};
static_assert(sizeof(Matrix3) == 48);
static_assert(std::is_trivially_copyable<Matrix3>());
static_assert(std::is_standard_layout<Matrix3>());
template <>
struct MatrixTypes<3, 4> {
  using Type = Matrix3x4;
};

union Matrix4 {
  Vector4 rows[4];
  float data[16];

  inline constexpr Vector4& operator[](int row_index) { return rows[row_index]; }
  inline constexpr Vector4 operator[](int row_index) const { return rows[row_index]; }

  static constexpr int ELEMENT_COUNT = 4;
  static constexpr int ROW_COUNT = 4;
  static constexpr int COLUMN_COUNT = 4;

  inline static constexpr Matrix4 Identity() {
    return {{
        // clang-format off
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        // clang-format on
    }};
  }

  inline static constexpr Matrix4 FromTranslation(const Vector3& translation);
  inline static constexpr Matrix4 FromScaling(const Vector3& scaling);
  inline static constexpr Matrix4 FromScaling(float scaling);
  inline static constexpr Matrix4 FromRotation(const Quaternion& rotation);
  inline static constexpr Matrix4 FromOrthographicProjection(float left, float right, float bottom, float top,
                                                             float near_clip_plane, float far_clip_plane);
  inline static Matrix4 FromPerspectiveProjection(float vertical_field_of_view, float aspect_ratio,
                                                  float near_clip_plane, float far_clip_plane);
};
static_assert(sizeof(Matrix4) == 64);
static_assert(std::is_trivially_copyable<Matrix4>());
static_assert(std::is_standard_layout<Matrix4>());
template <>
struct MatrixTypes<4, 4> {
  using Type = Matrix4;
};

template <int ROW_COUNT, int COLUMN_COUNT>
using Matrix = typename MatrixTypes<ROW_COUNT, COLUMN_COUNT>::Type;

template <typename T>
struct is_matrix : public std::integral_constant<bool, std::is_same<T, Matrix2>() || std::is_same<T, Matrix3>() ||
                                                           std::is_same<T, Matrix3x4>() || std::is_same<T, Matrix4>()> {
};

}  // namespace ovis

template <>
struct fmt::formatter<ovis::Matrix2> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::Matrix2& matrix, FormatContext& ctx) {
    for (int i = 0; i < ovis::Matrix2::ROW_COUNT; ++i) {
      format_to(ctx.out(), "\n{}", matrix[i]);
    }
    return ctx.out();
  }
};

template <>
struct fmt::formatter<ovis::Matrix3> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::Matrix3& matrix, FormatContext& ctx) {
    for (int i = 0; i < ovis::Matrix3::ROW_COUNT; ++i) {
      format_to(ctx.out(), "\n{}", matrix[i]);
    }
    return ctx.out();
  }
};

template <>
struct fmt::formatter<ovis::Matrix3x4> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::Matrix3x4& matrix, FormatContext& ctx) {
    for (int i = 0; i < ovis::Matrix3x4::ROW_COUNT; ++i) {
      format_to(ctx.out(), "\n{}", matrix[i]);
    }
    return ctx.out();
  }
};

template <>
struct fmt::formatter<ovis::Matrix4> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::Matrix4& matrix, FormatContext& ctx) {
    for (int i = 0; i < ovis::Matrix4::ROW_COUNT; ++i) {
      format_to(ctx.out(), "\n{}", matrix[i]);
    }
    return ctx.out();
  }
};
