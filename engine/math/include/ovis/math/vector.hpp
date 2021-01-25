#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <ovis/core/json.hpp>

namespace nlohmann {

template <>
struct adl_serializer<glm::vec2> {
  static void to_json(json& json, const glm::vec2& vector) { json = {vector[0], vector[1]}; }

  static void from_json(const json& json, glm::vec2& vector) {
    for (int i = 0; i < 2; ++i) {
      json.at(i).get_to(vector[i]);
    }
  }
};

template <>
struct adl_serializer<glm::vec3> {
  static void to_json(json& json, const glm::vec3& vector) { json = {vector[0], vector[1], vector[2]}; }

  static void from_json(const json& json, glm::vec3& vector) {
    for (int i = 0; i < 3; ++i) {
      json.at(i).get_to(vector[i]);
    }
  }
};

template <>
struct adl_serializer<glm::vec4> {
  static void to_json(json& json, const glm::vec4& vector) { json = {vector[0], vector[1], vector[2], vector[3]}; }

  static void from_json(const json& json, glm::vec4& vector) {
    for (int i = 0; i < 4; ++i) {
      json.at(i).get_to(vector[i]);
    }
  }
};

}  // namespace nlohmann
