#pragma once

#include <cmath>
#include <type_traits>

#include <fmt/format.h>
#include <sol/sol.hpp>

#include <ovis/utils/json.hpp>
#include <ovis/core/math_constants.hpp>

namespace ovis {

namespace vm {
class Module;
}

template <int ELEMENT_COUNT>
struct VectorTypes;

union alignas(sizeof(float) * 2) Vector2 {
  struct {
    float x;
    float y;
  };
  float data[2];

  inline Vector2() = default;
  inline constexpr Vector2(float x, float y) : x(x), y(y) {}
  inline float& operator[](int index) { return data[index]; }
  inline float operator[](int index) const { return data[index]; }

  static constexpr int ELEMENT_COUNT = 2;

  inline static constexpr Vector2 Zero() { return {0.0f, 0.0f}; }
  inline static constexpr Vector2 One() { return {1.0f, 1.0f}; }
  inline static constexpr Vector2 Infinity() { return {ovis::Infinity<float>(), ovis::Infinity<float>()}; }
  inline static constexpr Vector2 NotANumber() { return {ovis::NotANumber<float>(), ovis::NotANumber<float>()}; }
  inline static constexpr Vector2 PositiveX() { return {1.0f, 0.0f}; }
  inline static constexpr Vector2 NegativeX() { return {-1.0f, 0.0f}; }
  inline static constexpr Vector2 PositiveY() { return {0.0f, 1.0f}; }
  inline static constexpr Vector2 NegativeY() { return {0.0f, -1.0f}; }

  // Represents the coordinates of a unit square centered at the origin. Thus, all the absolut value of all component
  // in all vectors is 0.5 and the index in binary indicates the sign of the component. If the LSB is zero the sign
  // of the x component is negative, otherwise its positive and so on.
  inline static constexpr std::array<Vector2, 4> UnitSquare() {
    return {{
        {-0.5f, -0.5f},
        {+0.5f, -0.5f},
        {-0.5f, +0.5f},
        {+0.5f, +0.5f},
    }};
  };

  static void RegisterType(sol::table* module);
  static void RegisterType(vm::Module* module);
};
static_assert(sizeof(Vector2) == 8);
static_assert(std::is_trivially_copyable<Vector2>());
static_assert(std::is_standard_layout<Vector2>());
template <>
struct VectorTypes<2> {
  using Type = Vector2;
};

union alignas(sizeof(float) * 4) Vector3 {
  struct {
    float x;
    float y;
    float z;
  };
  float data[3];

  inline Vector3() = default;
  inline constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
  inline constexpr float& operator[](int index) { return data[index]; }
  inline constexpr float operator[](int index) const { return data[index]; }
  inline constexpr operator Vector2() const { return {x, y}; }

  static constexpr int ELEMENT_COUNT = 3;

  inline static constexpr Vector3 FromVector2(Vector2 v, float z = 0.0f) { return {v.x, v.y, z}; }

  inline static constexpr Vector3 Zero() { return {0.0f, 0.0f, 0.0f}; }
  inline static constexpr Vector3 One() { return {1.0f, 1.0f, 1.0f}; }
  inline static constexpr Vector3 Infinity() {
    return {ovis::Infinity<float>(), ovis::Infinity<float>(), ovis::Infinity<float>()};
  }
  inline static constexpr Vector3 NotANumber() {
    return {ovis::NotANumber<float>(), ovis::NotANumber<float>(), ovis::NotANumber<float>()};
  }
  inline static constexpr Vector3 PositiveX() { return {1.0f, 0.0f, 0.0f}; }
  inline static constexpr Vector3 NegativeX() { return {-1.0f, 0.0f, 0.0f}; }
  inline static constexpr Vector3 PositiveY() { return {0.0f, 1.0f, 0.0f}; }
  inline static constexpr Vector3 NegativeY() { return {0.0f, -1.0f, 0.0f}; }
  inline static constexpr Vector3 PositiveZ() { return {0.0f, 0.0f, 1.0f}; }
  inline static constexpr Vector3 NegativeZ() { return {0.0f, 0.0f, -1.0f}; }

  // Represents the coordinates of a unit cube centered at the origin. Thus, all the absolut value of all component
  // in all vectors is 0.5 and the index in binary indicates the sign of the component. If the LSB is zero the sign
  // of the x component is negative, otherwise its positive and so on.
  inline static constexpr std::array<Vector3, 8> UnitCube() {
    return {{
        {-0.5f, -0.5f, -0.5f},
        {+0.5f, -0.5f, -0.5f},
        {-0.5f, +0.5f, -0.5f},
        {+0.5f, +0.5f, -0.5f},
        {-0.5f, -0.5f, +0.5f},
        {+0.5f, -0.5f, +0.5f},
        {-0.5f, +0.5f, +0.5f},
        {+0.5f, +0.5f, +0.5f},
    }};
  };

  static void RegisterType(sol::table* module);
  static void RegisterType(vm::Module* module);
};
std::ostream& operator<<(std::ostream& stream, const Vector3& vector);
static_assert(sizeof(Vector3) == 16);
static_assert(std::is_trivially_copyable<Vector3>());
static_assert(std::is_standard_layout<Vector3>());
template <>
struct VectorTypes<3> {
  using Type = Vector3;
};

union alignas(sizeof(float) * 4) Vector4 {
  struct {
    float x;
    float y;
    float z;
    float w;
  };
  float data[4];

  inline Vector4() = default;
  inline constexpr Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
  inline constexpr float& operator[](int index) { return data[index]; }
  inline constexpr float operator[](int index) const { return data[index]; }
  inline constexpr operator Vector2() const { return {x, y}; }
  inline constexpr operator Vector3() const { return {x, y, z}; }

  static constexpr int ELEMENT_COUNT = 4;

  inline static constexpr Vector4 FromVector2(Vector2 v, float z = 0.0f, float w = 0.0f) { return {v.x, v.y, z, w}; }
  inline static constexpr Vector4 FromVector3(Vector3 v, float w = 0.0f) { return {v.x, v.y, v.z, w}; }

  inline static constexpr Vector4 Zero() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
  inline static constexpr Vector4 One() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
  inline static constexpr Vector4 Infinity() {
    return {ovis::Infinity<float>(), ovis::Infinity<float>(), ovis::Infinity<float>(), ovis::Infinity<float>()};
  }
  inline static constexpr Vector4 NotANumber() {
    return {ovis::NotANumber<float>(), ovis::NotANumber<float>(), ovis::NotANumber<float>(), ovis::NotANumber<float>()};
  }
  inline static constexpr Vector4 PositiveX() { return {1.0f, 0.0f, 0.0f, 0.0f}; }
  inline static constexpr Vector4 NegativeX() { return {-1.0f, 0.0f, 0.0f, 0.0f}; }
  inline static constexpr Vector4 PositiveY() { return {0.0f, 1.0f, 0.0f, 0.0f}; }
  inline static constexpr Vector4 NegativeY() { return {0.0f, -1.0f, 0.0f, 0.0f}; }
  inline static constexpr Vector4 PositiveZ() { return {0.0f, 0.0f, 1.0f, 0.0f}; }
  inline static constexpr Vector4 NegativeZ() { return {0.0f, 0.0f, -1.0f, 0.0f}; }
  inline static constexpr Vector4 PositiveW() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
  inline static constexpr Vector4 NegativeW() { return {0.0f, 0.0f, 0.0f, -1.0f}; }

  // Represents the coordinates of a unit hypercube centered at the origin. Thus, all the absolut value of all component
  // in all vectors is 0.5 and the index in binary indicates the sign of the component. If the LSB is zero the sign
  // of the x component is negative, otherwise its positive and so on.
  inline static constexpr std::array<Vector4, 16> UnitHypercube() {
    return {{
        {-0.5f, -0.5f, -0.5f, -0.5f},
        {+0.5f, -0.5f, -0.5f, -0.5f},
        {-0.5f, +0.5f, -0.5f, -0.5f},
        {+0.5f, +0.5f, -0.5f, -0.5f},
        {-0.5f, -0.5f, +0.5f, -0.5f},
        {+0.5f, -0.5f, +0.5f, -0.5f},
        {-0.5f, +0.5f, +0.5f, -0.5f},
        {+0.5f, +0.5f, +0.5f, -0.5f},
        {-0.5f, -0.5f, -0.5f, +0.5f},
        {+0.5f, -0.5f, -0.5f, +0.5f},
        {-0.5f, +0.5f, -0.5f, +0.5f},
        {+0.5f, +0.5f, -0.5f, +0.5f},
        {-0.5f, -0.5f, +0.5f, +0.5f},
        {+0.5f, -0.5f, +0.5f, +0.5f},
        {-0.5f, +0.5f, +0.5f, +0.5f},
        {+0.5f, +0.5f, +0.5f, +0.5f},
    }};
  };
};
static_assert(sizeof(Vector4) == 16);
static_assert(std::is_trivially_copyable<Vector4>());
static_assert(std::is_standard_layout<Vector4>());
template <>
struct VectorTypes<4> {
  using Type = Vector4;
};

template <int ELEMENT_COUNT>
using Vector = typename VectorTypes<ELEMENT_COUNT>::Type;

template <typename T>
struct is_vector : public std::integral_constant<bool, std::is_same<T, Vector2>() || std::is_same<T, Vector3>() ||
                                                           std::is_same<T, Vector4>()> {};

}  // namespace ovis

namespace fmt {

template <>
struct fmt::formatter<ovis::Vector2> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::Vector2& vector, FormatContext& ctx) {
    return format_to(ctx.out(), presentation == 'f' ? "({:.1f}, {:.1f})" : "({:.1e}, {:.1e})", vector.x, vector.y);
  }
};

template <>
struct fmt::formatter<ovis::Vector3> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::Vector3& vector, FormatContext& ctx) {
    return format_to(ctx.out(), presentation == 'f' ? "({:.1f}, {:.1f}, {:.1f})" : "({:.1e}, {:.1e}, {:.1e})", vector.x,
                     vector.y, vector.z);
  }
};

template <>
struct fmt::formatter<ovis::Vector4> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::Vector4& vector, FormatContext& ctx) {
    return format_to(ctx.out(),
                     presentation == 'f' ? "({:.1f}, {:.1f}, {:.1f}, {:.1f})" : "({:.1e}, {:.1e}, {:.1e}, {:.1e})",
                     vector.x, vector.y, vector.z, vector.w);
  }
};

}  // namespace fmt

namespace nlohmann {

template <>
struct adl_serializer<ovis::Vector2> {
  static void to_json(json& json, const ovis::Vector2& vector) { json = {vector[0], vector[1]}; }

  static void from_json(const json& json, ovis::Vector2& vector) {
    for (int i = 0; i < 2; ++i) {
      json.at(i).get_to(vector[i]);
    }
  }
};

template <>
struct adl_serializer<ovis::Vector3> {
  static void to_json(json& json, const ovis::Vector3& vector) { json = {vector[0], vector[1], vector[2]}; }

  static void from_json(const json& json, ovis::Vector3& vector) {
    for (int i = 0; i < 3; ++i) {
      json.at(i).get_to(vector[i]);
    }
  }
};

template <>
struct adl_serializer<ovis::Vector4> {
  static void to_json(json& json, const ovis::Vector4& vector) { json = {vector[0], vector[1], vector[2], vector[3]}; }

  static void from_json(const json& json, ovis::Vector4& vector) {
    for (int i = 0; i < 4; ++i) {
      json.at(i).get_to(vector[i]);
    }
  }
};

}  // namespace nlohmann
