#pragma once

#include <array>
#include <type_traits>

#include <ovis/core/color.hpp>
#include <ovis/core/matrix.hpp>
#include <ovis/core/vector.hpp>
#include <ovis/graphics/gl.hpp>

namespace ovis {

// using Vec2 = std::array<float, 2>;
// using Vec3 = std::array<float, 3>;
// using Vec4 = std::array<float, 4>;

// using IntVec2 = std::array<int, 2>;
// using IntVec3 = std::array<int, 3>;
// using IntVec4 = std::array<int, 4>;

// using BoolVec2 = std::array<bool, 2>;
// using BoolVec3 = std::array<bool, 3>;
// using BoolVec4 = std::array<bool, 4>;

// using Mat2x2 = std::array<std::array<float, 2>, 2>;
// using Mat3x3 = std::array<std::array<float, 3>, 3>;
// using Mat4x4 = std::array<std::array<float, 4>, 4>;

template <typename T>
struct OpenGLTypeHelper;

#define OVIS_OPENGL_TYPE(cpp_type, opengl_type)  \
  template <>                                    \
  struct OpenGLTypeHelper<cpp_type> {            \
    static constexpr GLenum value = opengl_type; \
  }

OVIS_OPENGL_TYPE(float, GL_FLOAT);
OVIS_OPENGL_TYPE(Vector2, GL_FLOAT_VEC2);
OVIS_OPENGL_TYPE(Vector3, GL_FLOAT_VEC3);
OVIS_OPENGL_TYPE(Vector4, GL_FLOAT_VEC4);
OVIS_OPENGL_TYPE(Color, GL_FLOAT_VEC4);

OVIS_OPENGL_TYPE(int, GL_INT);
// OVIS_OPENGL_TYPE(IntVec2, GL_INT_VEC2);
// OVIS_OPENGL_TYPE(IntVec3, GL_INT_VEC3);
// OVIS_OPENGL_TYPE(IntVec4, GL_INT_VEC4);

OVIS_OPENGL_TYPE(bool, GL_BOOL);
// OVIS_OPENGL_TYPE(BoolVec2, GL_BOOL_VEC2);
// OVIS_OPENGL_TYPE(BoolVec3, GL_BOOL_VEC3);
// OVIS_OPENGL_TYPE(BoolVec4, GL_BOOL_VEC4);

OVIS_OPENGL_TYPE(Matrix2, GL_FLOAT_MAT2);
OVIS_OPENGL_TYPE(Matrix3, GL_FLOAT_MAT3);
OVIS_OPENGL_TYPE(Matrix4, GL_FLOAT_MAT4);

template <typename T>
inline constexpr GLenum OpenGLType = OpenGLTypeHelper<std::remove_cv_t<T>>::value;

}  // namespace ovis
