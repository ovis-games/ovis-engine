#pragma once

#include <fmt/format.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <ovis/core/json.hpp>

namespace ovis {

using vector2 = glm::vec2;
using vector3 = glm::vec3;
using vector4 = glm::vec4;
using matrix4 = glm::mat4;
using quaternion = glm::quat;

}  // namespace ovis

namespace fmt {

template <>
struct fmt::formatter<ovis::vector2> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::vector2& vector, FormatContext& ctx) {
    return format_to(ctx.out(), presentation == 'f' ? "({:.1f}, {:.1f})" : "({:.1e}, {:.1e})", vector.x, vector.y);
  }
};

template <>
struct fmt::formatter<ovis::vector3> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::vector3& vector, FormatContext& ctx) {
    return format_to(ctx.out(), presentation == 'f' ? "({:.1f}, {:.1f}, {:.1f})" : "({:.1e}, {:.1e}, {:.1e})", vector.x, vector.y, vector.z);
  }
};

template <>
struct fmt::formatter<ovis::vector4> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::vector4& vector, FormatContext& ctx) {
    return format_to(ctx.out(), presentation == 'f' ? "({:.1f}, {:.1f}, {:.1f}, {:.1f})" : "({:.1e}, {:.1e}, {:.1e}, {:.1e})", vector.x, vector.y, vector.z, vector.w);
  }
};

}

namespace nlohmann {

template <>
struct adl_serializer<ovis::vector2> {
  static void to_json(json& json, const ovis::vector2& vector) { json = {vector[0], vector[1]}; }

  static void from_json(const json& json, ovis::vector2& vector) {
    for (int i = 0; i < 2; ++i) {
      json.at(i).get_to(vector[i]);
    }
  }
};

template <>
struct adl_serializer<ovis::vector3> {
  static void to_json(json& json, const ovis::vector3& vector) { json = {vector[0], vector[1], vector[2]}; }

  static void from_json(const json& json, ovis::vector3& vector) {
    for (int i = 0; i < 3; ++i) {
      json.at(i).get_to(vector[i]);
    }
  }
};

template <>
struct adl_serializer<ovis::vector4> {
  static void to_json(json& json, const ovis::vector4& vector) { json = {vector[0], vector[1], vector[2], vector[3]}; }

  static void from_json(const json& json, ovis::vector4& vector) {
    for (int i = 0; i < 4; ++i) {
      json.at(i).get_to(vector[i]);
    }
  }
};

template <>
struct adl_serializer<ovis::matrix4> {
  static void to_json(json& json, const ovis::matrix4& matrix) {
    json = {matrix[0], matrix[1], matrix[2],  matrix[3],  matrix[4],  matrix[5],  matrix[6],  matrix[7],
            matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]};
  }

  static void from_json(const json& json, ovis::matrix4& matrix) {
    for (int i = 0; i < 16; ++i) {
      json.at(i).get_to(matrix[i]);
    }
  }
};

}  // namespace nlohmann
