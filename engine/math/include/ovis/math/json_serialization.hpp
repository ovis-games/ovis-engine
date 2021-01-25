#pragma once

#include <glm/mat4x4.hpp>
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

template <>
struct adl_serializer<glm::mat4> {
  static void to_json(json& json, const glm::mat4& matrix) {
    json = {matrix[0], matrix[1], matrix[2],  matrix[3],  matrix[4],  matrix[5],  matrix[6],  matrix[7],
            matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]};
  }

  static void from_json(const json& json, glm::mat4& matrix) {
    for (int i = 0; i < 16; ++i) {
      json.at(i).get_to(matrix[i]);
    }
  }
};

}  // namespace nlohmann
