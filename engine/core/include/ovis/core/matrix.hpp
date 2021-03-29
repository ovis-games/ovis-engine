#pragma once

#include <cmath>
#include <type_traits>

#include <fmt/format.h>

#include <ovis/core/matrix_types.hpp>
#include <ovis/core/vector.hpp>

namespace ovis {

inline constexpr float Determinant(const Matrix2& matrix) {
  return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
}

inline constexpr Matrix3x4 Matrix3x4::FromTransformation(const Vector3& translation, const Vector3& scaling,
                                                         const Quaternion& rotation) {
  Matrix3x4 transformation_matrix = Matrix3x4::FromRotation(rotation);
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) {
      transformation_matrix[row][column] *= scaling[column];
    }
    transformation_matrix[row][3] = translation[row];
  }
  return transformation_matrix;
}

inline constexpr Matrix3x4 Matrix3x4::FromTranslation(const Vector3& translation) {
  return {{
      // clang-format off
      {1.0f, 0.0f, 0.0f, translation.x},
      {0.0f, 1.0f, 0.0f, translation.y},
      {0.0f, 0.0f, 1.0f, translation.z},
      // clang-format on
  }};
}

inline constexpr Matrix3x4 Matrix3x4::FromScaling(const Vector3& scaling) {
  return {{
      // clang-format off
      {scaling.x, 0.0f, 0.0f, 0.0f},
      {0.0f, scaling.y, 0.0f, 0.0f},
      {0.0f, 0.0f, scaling.z, 0.0f},
      // clang-format on
  }};
}

inline constexpr Matrix3x4 Matrix3x4::FromScaling(float scaling) {
  return {{
      // clang-format off
      {scaling, 0.0f, 0.0f, 0.0f},
      {0.0f, scaling, 0.0f, 0.0f},
      {0.0f, 0.0f, scaling, 0.0f},
      // clang-format on
  }};
}

inline constexpr Matrix3x4 Matrix3x4::FromRotation(const Quaternion& q) {
  const float qxx = q.x * q.x;
  const float qyy = q.y * q.y;
  const float qzz = q.z * q.z;
  const float qxz = q.x * q.z;
  const float qxy = q.x * q.y;
  const float qyz = q.y * q.z;
  const float qwx = q.w * q.x;
  const float qwy = q.w * q.y;
  const float qwz = q.w * q.z;

  const float m00 = 1.0f - 2.0f * (qyy + qzz);
  const float m10 = 2.0f * (qxy + qwz);
  const float m20 = 2.0f * (qxz - qwy);

  const float m01 = 2.0f * (qxy - qwz);
  const float m11 = 1.0f - 2.0f * (qxx + qzz);
  const float m21 = 2.0f * (qyz + qwx);

  const float m02 = 2.0f * (qxz + qwy);
  const float m12 = 2.0f * (qyz - qwx);
  const float m22 = 1.0f - 2.0f * (qxx + qyy);

  return {{
      // clang-format off
      {m00, m01, m02, 0.f},
      {m10, m11, m12, 0.f},
      {m20, m21, m22, 0.f},
      // clang-format on
  }};
}

inline constexpr Matrix4 Matrix4::FromTranslation(const Vector3& translation) {
  return {{
      // clang-format off
      {1.0f, 0.0f, 0.0f, translation.x},
      {0.0f, 1.0f, 0.0f, translation.y},
      {0.0f, 0.0f, 1.0f, translation.z},
      {0.0f, 0.0f, 0.0f, 1.0f},
      // clang-format on
  }};
}

inline constexpr Matrix4 Matrix4::FromScaling(const Vector3& scaling) {
  return {{
      // clang-format off
      {scaling.x, 0.0f, 0.0f, 0.0f},
      {0.0f, scaling.y, 0.0f, 0.0f},
      {0.0f, 0.0f, scaling.z, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
      // clang-format on
  }};
}

inline constexpr Matrix4 Matrix4::FromScaling(float scaling) {
  return {{
      // clang-format off
      {scaling, 0.0f, 0.0f, 0.0f},
      {0.0f, scaling, 0.0f, 0.0f},
      {0.0f, 0.0f, scaling, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
      // clang-format on
  }};
}

inline constexpr Matrix4 Matrix4::FromRotation(const Quaternion& q) {
  const float qxx = q.x * q.x;
  const float qyy = q.y * q.y;
  const float qzz = q.z * q.z;
  const float qxz = q.x * q.z;
  const float qxy = q.x * q.y;
  const float qyz = q.y * q.z;
  const float qwx = q.w * q.x;
  const float qwy = q.w * q.y;
  const float qwz = q.w * q.z;

  const float m00 = 1.0f - 2.0f * (qyy + qzz);
  const float m10 = 2.0f * (qxy + qwz);
  const float m20 = 2.0f * (qxz - qwy);

  const float m01 = 2.0f * (qxy - qwz);
  const float m11 = 1.0f - 2.0f * (qxx + qzz);
  const float m21 = 2.0f * (qyz + qwx);

  const float m02 = 2.0f * (qxz + qwy);
  const float m12 = 2.0f * (qyz - qwx);
  const float m22 = 1.0f - 2.0f * (qxx + qyy);

  return {{
      // clang-format off
      {m00, m01, m02, 0.f},
      {m10, m11, m12, 0.f},
      {m20, m21, m22, 0.f},
      {0.f, 0.f, 0.f, 1.f},
      // clang-format on
  }};
}
inline constexpr Matrix4 Matrix4::FromOrthographicProjection(float left, float right, float bottom, float top,
                                                             float near_clip_plane, float far_clip_plane) {
  const float m00 = 2.0f / (right - left);
  const float m03 = -(right + left) / (right - left);
  const float m11 = 2.0f / (top - bottom);
  const float m13 = -(top + bottom) / (top - bottom);
  const float m22 = 1.0f / (far_clip_plane - near_clip_plane);
  const float m23 = -near_clip_plane / (far_clip_plane - near_clip_plane);

  return {{
      {m00, 0.f, 0.f, m03},
      {0.f, m11, 0.f, m13},
      {0.f, 0.f, m22, m23},
      {0.f, 0.f, 0.f, 1.f},
  }};
}

inline constexpr Matrix4 Matrix4::FromPerspectiveProjection(float vertical_field_of_view, float aspect_ratio,
                                                            float near_clip_plane, float far_clip_plane) {
  const float tan_half_fov = tan(0.5f * vertical_field_of_view);

  const float m00 = 1.0f / (aspect_ratio * tan_half_fov);
  const float m11 = 1.0f / (tan_half_fov);
  const float m22 = (far_clip_plane + near_clip_plane) / (far_clip_plane - near_clip_plane);
  const float m23 = -(2.0f * far_clip_plane * near_clip_plane) / (far_clip_plane - near_clip_plane);

  return {{
      {m00, 0.f, 0.f, 0.f},
      {0.f, m11, 0.f, 0.f},
      {0.f, 0.f, m22, m23},
      {0.f, 0.f, 1.f, 0.f},
  }};
}

template <typename T, std::enable_if_t<is_matrix<T>::value, bool> = true>
inline constexpr auto ExtractColumn(const T& matrix, int column_index) {
  Vector<T::ROW_COUNT> vector;
  for (int i = 0; i < T::ROW_COUNT; ++i) {
    vector[i] = matrix[i][column_index];
  }
  return vector;
}

template <typename T, std::enable_if_t<is_matrix<T>::value, bool> = true>
inline constexpr auto Transpose(const T& matrix) {
  Matrix<T::COLUMN_COUNT, T::ROW_COUNT> result;
  for (int i = 0; i < T::ROW_COUNT; ++i) {
    for (int j = 0; j < T::COLUMN_COUNT; ++j) {
      result[i][j] = matrix[j][i];
    }
  }
  return result;
}

template <typename T, typename U,
          std::enable_if_t<is_matrix<T>::value && is_matrix<U>::value && U::COLUMN_COUNT == T::ROW_COUNT, bool> = true>
inline constexpr Matrix<U::ROW_COUNT, T::COLUMN_COUNT> operator*(const T& lhs, const U& rhs) {
  Matrix<U::ROW_COUNT, T::COLUMN_COUNT> result;
  for (int i = 0; i < T::ROW_COUNT; ++i) {
    for (int j = 0; j < T::COLUMN_COUNT; ++j) {
      result[i][j] = Dot(lhs[i], ExtractColumn(rhs, j));
    }
  }
  return result;
}

// Special "multiplication" for affine 3x4 matrices. Multiplies a 3x4 or a 4x4 and a 3x4 matrix but assumes
// that the 3x4 matrix on the right is actually a 4x4 matrix with the last row set to (0, 0, 0, 1). This allows
// combining several transformations stored in 3x4 matrices.
template <typename T, std::enable_if_t<is_matrix<T>::value && T::COLUMN_COUNT == 4, bool> = true>
inline constexpr auto AffineCombine(const T& lhs, const Matrix3x4& rhs) {
  Matrix<T::ROW_COUNT, 4> result;
  for (int i = 0; i < T::ROW_COUNT; ++i) {
    for (int j = 0; j < 3; ++j) {
      result[i][j] = Dot(lhs[i], Vector4::FromVector3(ExtractColumn(rhs, j), 0.0f));
    }
    result[i][3] = Dot(lhs[i], Vector4::FromVector3(ExtractColumn(rhs, 3), 1.0f));
  }
  return result;
}

template <typename T, std::enable_if_t<is_matrix<T>::value, bool> = true>
inline constexpr T operator*=(T& lhs, const T& rhs) {
  return lhs = lhs * rhs;
}

template <typename MatrixType, typename VectorType,
          std::enable_if_t<is_matrix<MatrixType>::value && is_vector<VectorType>::value &&
                               MatrixType::COLUMN_COUNT == VectorType::ELEMENT_COUNT,
                           bool> = true>
inline constexpr Vector<MatrixType::ROW_COUNT> operator*(const MatrixType& matrix, const VectorType& vector) {
  Vector<MatrixType::ROW_COUNT> result;
  for (int i = 0; i < MatrixType::ROW_COUNT; ++i) {
    result[i] = Dot(matrix[i], vector);
  }
  return result;
}

template <typename MatrixType, typename VectorType,
          std::enable_if_t<is_matrix<MatrixType>::value && is_vector<VectorType>::value &&
                               MatrixType::ROW_COUNT == VectorType::ELEMENT_COUNT,
                           bool> = true>
inline constexpr Vector<MatrixType::COLUMN_COUNT> operator*(const VectorType& vector, const MatrixType& matrix) {
  Vector<MatrixType::COLUMN_COUNT> result;
  for (int i = 0; i < MatrixType::COLUMN_COUNT; ++i) {
    result[i] = Dot(vector, ExtractColumn(matrix, i));
  }
  return result;
}

inline constexpr Vector3 TransformDirection(const Matrix3x4& transform_matrix, const Vector3& vector) {
  const Vector4 direction_vector = {vector.x, vector.y, vector.z, 0.0f};
  return transform_matrix * direction_vector;
}

inline constexpr Vector3 TransformPosition(const Matrix3x4& transform_matrix, const Vector3& vector) {
  const Vector4 position_vector = {vector.x, vector.y, vector.z, 1.0f};
  return transform_matrix * position_vector;
}

inline constexpr Vector3 TransformPosition(const Matrix4& transform_matrix, const Vector3& vector) {
  const Vector4 position_vector = {vector.x, vector.y, vector.z, 1.0f};
  const Vector4 transformed_vector = transform_matrix * position_vector;
  return Vector3{transformed_vector.x, transformed_vector.y, transformed_vector.z} / transformed_vector.w;
}

inline constexpr Matrix3x4 InvertAffineNoScale(const Matrix3x4& matrix) {
  // See https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html for an explanation.
  // Note here the translation is in the last column, not the last row!.
  Matrix3x4 result;

  const Vector3 translation = ExtractColumn(matrix, 3);
  for (int i = 0; i < 3; ++i) {
    const Vector3 column = ExtractColumn(matrix, i);
    result[i] = Vector4::FromVector3(column, -Dot(column, translation));
  }

  return result;
}

inline constexpr Matrix3x4 InvertAffine(const Matrix3x4& matrix) {
  // Same as above, but we'll take the scaling into account.
  Matrix3x4 result;

  const Vector3 translation = ExtractColumn(matrix, 3);
  for (int i = 0; i < 3; ++i) {
    const Vector3 column = ExtractColumn(matrix, i);
    const float scale = SquaredLength(column);
    result[i] = (1.0f / scale) * Vector4::FromVector3(column, -Dot(column, translation));
  }
  return result;
}

inline constexpr Matrix4 Invert(const Matrix4& matrix) {
  // TODO: implement the method explained here:
  // https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html

  // First: split matrix into 4 2x2 matrices:
  // / a b \
  // \ c d /
  // const Matrix2 a = Matrix2::FromVector(Shuffle<0, 1, 0, 1>(matrix[0], matrix[1]));
  // const Matrix2 b = Matrix2::FromVector(Shuffle<2, 3, 2, 3>(matrix[0], matrix[1]));
  // const Matrix2 c = Matrix2::FromVector(Shuffle<0, 1, 0, 1>(matrix[2], matrix[3]));
  // const Matrix2 d = Matrix2::FromVector(Shuffle<2, 3, 2, 3>(matrix[2], matrix[3]));

  // const float determinant_a = Determinant(a);
  // const float determinant_b = Determinant(b);
  // const float determinant_b = Determinant(b);
  // const float determinant_b = Determinant(b);

  float m00 = matrix[0][0], m01 = matrix[0][1], m02 = matrix[0][2], m03 = matrix[0][3];
  float m10 = matrix[1][0], m11 = matrix[1][1], m12 = matrix[1][2], m13 = matrix[1][3];
  float m20 = matrix[2][0], m21 = matrix[2][1], m22 = matrix[2][2], m23 = matrix[2][3];
  float m30 = matrix[3][0], m31 = matrix[3][1], m32 = matrix[3][2], m33 = matrix[3][3];

  float v0 = m20 * m31 - m21 * m30;
  float v1 = m20 * m32 - m22 * m30;
  float v2 = m20 * m33 - m23 * m30;
  float v3 = m21 * m32 - m22 * m31;
  float v4 = m21 * m33 - m23 * m31;
  float v5 = m22 * m33 - m23 * m32;

  float t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
  float t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
  float t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
  float t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

  float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

  float d00 = t00 * invDet;
  float d10 = t10 * invDet;
  float d20 = t20 * invDet;
  float d30 = t30 * invDet;

  float d01 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
  float d11 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
  float d21 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
  float d31 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

  v0 = m10 * m31 - m11 * m30;
  v1 = m10 * m32 - m12 * m30;
  v2 = m10 * m33 - m13 * m30;
  v3 = m11 * m32 - m12 * m31;
  v4 = m11 * m33 - m13 * m31;
  v5 = m12 * m33 - m13 * m32;

  float d02 = +(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
  float d12 = -(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
  float d22 = +(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
  float d32 = -(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

  v0 = m21 * m10 - m20 * m11;
  v1 = m22 * m10 - m20 * m12;
  v2 = m23 * m10 - m20 * m13;
  v3 = m22 * m11 - m21 * m12;
  v4 = m23 * m11 - m21 * m13;
  v5 = m23 * m12 - m22 * m13;

  float d03 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
  float d13 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
  float d23 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
  float d33 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

  return {{
      {d00, d01, d02, d03},
      {d10, d11, d12, d13},
      {d20, d21, d22, d23},
      {d30, d31, d32, d33},
  }};
}

}  // namespace ovis
