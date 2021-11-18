#pragma once

#include <type_traits>

#include <fmt/format.h>
#include <sol/sol.hpp>

#include <ovis/utils/json.hpp>

namespace ovis {

union alignas(sizeof(float) * 4) Color {
  struct {
    float r;
    float g;
    float b;
    float a;
  };
  float data[4];

  inline float& operator[](int index) { return data[index]; }
  inline float operator[](int index) const { return data[index]; }

  static constexpr int ELEMENT_COUNT = 4;

  inline Color() = default;
  inline constexpr Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

  inline static constexpr Color Transparent() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
  inline static constexpr Color Black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
  inline static constexpr Color White() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
  inline static constexpr Color Red() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
  inline static constexpr Color Green() { return {0.0f, 1.0f, 0.0f, 1.0f}; }
  inline static constexpr Color Blue() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
  inline static constexpr Color Yellow() { return {1.0f, 1.0f, 0.0f, 1.0f}; }
  inline static constexpr Color Fuchsia() { return {1.0f, 0.0f, 1.0f, 1.0f}; }
  inline static constexpr Color Aqua() { return {0.0f, 1.0f, 1.0f, 1.0f}; }

  static void RegisterType(sol::table* module);
};
static_assert(sizeof(Color) == 16);
static_assert(std::is_trivially_copyable<Color>());
static_assert(std::is_standard_layout<Color>());

template <typename T>
struct is_color : public std::integral_constant<bool, std::is_same<T, Color>::value> {};

}  // namespace ovis

namespace fmt {

template <>
struct formatter<ovis::Color> {
  char presentation = 'f';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::Color& color, FormatContext& ctx) {
    return format_to(ctx.out(),
                     presentation == 'f' ? "({:.1f}, {:.1f}, {:.1f}, {:.1f})" : "({:.1e}, {:.1e}, {:.1e}, {:.1e})",
                     color.r, color.g, color.b, color.a);
  }
};

}  // namespace fmt

namespace nlohmann {

template <>
struct adl_serializer<ovis::Color> {
  static void to_json(json& json, const ovis::Color& color) { json = {color[0], color[1], color[2], color[3]}; }

  static void from_json(const json& json, ovis::Color& color) {
    for (int i = 0; i < 4; ++i) {
      json.at(i).get_to(color[i]);
    }
  }
};

}  // namespace nlohmann
