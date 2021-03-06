#pragma once

#include <cmath>
#include <type_traits>

#include <fmt/format.h>

#include <ovis/core/json.hpp>
#include <ovis/math/constants.hpp>
#include <ovis/math/quaternion_type.hpp>
#include <ovis/math/vector_types.hpp>

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

  inline static constexpr Matrix2 FromVector(const Vector4& vector);
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

  inline Vector3& operator[](int row_index) { return rows[row_index]; }
  inline Vector3 operator[](int row_index) const { return rows[row_index]; }

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

  inline Vector4& operator[](int row_index) { return rows[row_index]; }
  inline Vector4 operator[](int row_index) const { return rows[row_index]; }

  static constexpr int ELEMENT_COUNT = 3;
  static constexpr int ROW_COUNT = 3;
  static constexpr int COLUMN_COUNT = 4;
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

  inline Vector4& operator[](int row_index) { return rows[row_index]; }
  inline Vector4 operator[](int row_index) const { return rows[row_index]; }

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
  inline static constexpr Matrix4 FromPerspectiveProjection(float vertical_field_of_view, float aspect_ratio,
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
                                                           std::is_same<T, Matrix4>()> {};

}  // namespace ovis

namespace fmt {

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
    return format_to(
        ctx.out(),
        presentation == 'f' ? "\n({:.5f}, {:.5f}, {:.5f}, {:.5f})\n({:.5f}, {:.5f}, {:.5f}, {:.5f})\n({:.5f}, {:.5f}, "
                              "{:.5f}, {:.5f})\n({:.5f}, {:.5f}, {:.5f}, {:.5f})"
                            : "\n({:.1e}, {:.1e}, {:.1e}, {:.1e})\n({:.1e}, {:.1e}, {:.1e}, {:.1e})\n({:.1e}, {:.1e}, "
                              "{:.1e}, {:.1e})\n({:.1e}, {:.1e}, {:.1e}, {:.1e})",
        matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3], matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
        matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3], matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
  }
};

}  // namespace fmt
