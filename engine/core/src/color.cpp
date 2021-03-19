#include <ovis/core/color.hpp>

namespace ovis {

void Color::RegisterType(sol::table* module) {
  // clang-format off

  /// Represents a color using red, green, blue and alpa.
  // The value for r, g, b and a range from 0 to 1. E.g., the RGB values
  // 0,0,0 respresent black and 1,1,1 respresent white. Similarly, an
  // alpha value of 0 represents full transparency whereas an alpha value
  // of 1 represents full opaqueness.
  // @classmod ovis.core.Color
  // @usage local core = require "ovis.core"
  // local Color = core.Color
  sol::usertype<Color> color_type = module->new_usertype<Color>(
    "Color", sol::factories(
      []() { return Color(0, 0, 0, 1.0f); },
      [](float r, float g, float b) { return Color(r, g, b); },
      [](float r, float g, float b, float a) { return Color(r, g, b, a); },
      [](const sol::table& t) {
        return Color(
          t.get_or("r", t.get_or(1, 0.0f)),
          t.get_or("g", t.get_or(2, 0.0f)),
          t.get_or("b", t.get_or(3, 0.0f)),
          t.get_or("a", t.get_or(4, 1.0f))
        );
      }
    )
  );

  /// The red component of the vector.
  // @field[type=number] r
  color_type["r"] = &Color::r;

  /// The green component of the vector.
  // @field[type=number] g
  color_type["g"] = &Color::g;

  /// The blue component of the vector.
  // @field[type=number] b
  color_type["b"] = &Color::b;

  /// The alpha component of the vector.
  // @field[type=number] a
  color_type["a"] = &Color::a;

  /// Transparent color (0, 0, 0, 0)
  // @field[type=Color] TRANSPARENT
  color_type["TRANSPARENT"] = sol::property(Color::Transparent);

  /// Black color (0, 0, 0, 1)
  // @field[type=Color] BLACK
  color_type["BLACK"] = sol::property(Color::Black);

  /// White color (1, 1, 1, 1)
  // @field[type=Color] WHITE
  color_type["WHITE"] = sol::property(Color::White);

  /// Red color (1, 0, 0, 1)
  // @field[type=Color] RED
  color_type["RED"] = sol::property(Color::Red);

  /// Green color (0, 1, 0, 1)
  // @field[type=Color] GREEN
  color_type["GREEN"] = sol::property(Color::Green);

  /// Blue color (0, 0, 1, 1)
  // @field[type=Color] BLUE
  color_type["BLUE"] = sol::property(Color::Blue);

  /// Yellow color (1, 1, 0, 1)
  // @field[type=Color] YELLOW
  color_type["YELLOW"] = sol::property(Color::Yellow);

  /// Fuchsia color (1, 0, 1, 1)
  // @field[type=Color] FUCHSIA
  color_type["FUCHSIA"] = sol::property(Color::Fuchsia);

  /// Aqua color (0, 0, 1, 1)
  // @field[type=Color] AQUA
  color_type["AQUA"] = sol::property(Color::Aqua);

  /// Creates a new color.
  // The r, g and b component will be set to 0 and a will be set to 1.
  // @function new
  // @treturn Color
  // @usage local c = Color.new()
  // assert(c.r == 0 and c.g == 0 and c.b == 0 and c.a == 1)

  /// Creates a new color from the arguments.
  // @function new
  // @param[type=number] r The value for the red channel.
  // @param[type=number] g The value for the green channel.
  // @param[type=number] b The value for the blue channel.
  // @param[type=number,opt=1] a The value for the alpha channel.
  // @treturn Color
  // @usage local c = Color.new(1.0, 0.0, 0.5)
  // assert(c.r == 1 and c.g == 0 and c.b == 0.5 and c.a == 1)

  /// Creates a new color from the arguments.
  // @function new
  // @param[type=table] table
  // @treturn Color
  // @usage -- All the following calls create the same color:
  // local c1 = Color.new(1, 0.5, 1)
  // local c2 = Color.new(1, 0.5, 1, 1)
  // local c3 = Color.new{1, 0.5, 1}
  // local c4 = Color.new{r = 1, g = 0.5, b = 1}
  // assert(c1 == c2)
  // assert(c2 == c3)
  // assert(c3 == c4)

  /// Compares two colors for equality.
  //
  // Be very careful when comparing two colors as the components are floating point values!
  //
  // @function __eq
  // @param[type=Color] c1
  // @param[type=Color] c2
  // @treturn bool
  // @usage local c1 = Color.new(1.0, 2.0, 3.0)
  // local c2 = Color.new(1.0, 2.0, 3.0)
  // local c3 = Color.new(1.0, 0.0, 3.0)
  // assert(c1 == c2) -- c1 and c2 are equal
  // assert(c1 ~= c3) -- c1 and c3 are not
  color_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Color&, const Color&)>(ovis::operator==);

  /// Adds two colors.
  // @function __add
  // @param[type=Color] c1
  // @param[type=Color] c2
  // @treturn Color
  // @usage local c1 = Color.new(1, 0.5, 1)
  // local c2 = Color.new(0.5, 1, 0.5)
  // local c3 = c1 + c2
  // assert(c3 == Color.new(1.5, 1.5, 1.5, 2))
  color_type[sol::meta_function::addition] =
      static_cast<Color (*)(const Color&, const Color&)>(ovis::operator+);

  /// Negates a color.
  // Negative values for colors are not necessarily useful, however,
  // it make some algorithm more straightforward to implement.
  // @function __unm
  // @param[type=Color] c
  // @treturn Color
  // @usage local c1 = Color.new(1, -2, 3)
  // local c2 = -c1
  // assert(c2 == Color.new(-1, 2, -3, -1))
  color_type[sol::meta_function::unary_minus] = static_cast<Color (*)(const Color&)>(ovis::operator-);

  /// Subtracts two colors.
  // @function __sub
  // @param[type=Color] c1
  // @param[type=Color] c2
  // @treturn Color
  // @usage local c1 = Color.new(1, 0.5, 1)
  // local c2 = Color.new(0.5, 0.5, 0.5, 0)
  // local c3 = c1 - c2
  // assert(c3 == Color.new(0.5, 0, 0.5))
  color_type[sol::meta_function::subtraction] =
      static_cast<Color (*)(const Color&, const Color&)>(ovis::operator-);

  /// Multiplies two colors or a scalar and a color.
  // @function __mul
  // @param[type=Color|number] c1
  // @param[type=Color|number] c2
  // @treturn Color
  // @usage local c1 = Color.new(1, 0.5, 1)
  // local c2 = Color.new(0.5, 1, 0.5)
  // local c3 = c1 * c2 -- multiply two colors component-wise
  // assert(c3 == Color.new(0.5, 0.5, 0.5))
  // local v4 = c1 * 0.5 -- multiply a color and a scalar
  // assert(v4 == Color.new(0.5, 0.25, 0.5, 0.5))
  // local v5 = 0.5 * c1 -- you can also multiply from the other side
  // assert(v5 == Color.new(0.5, 0.25, 0.5, 0.5))
  color_type[sol::meta_function::multiplication] =
      sol::overload(static_cast<Color (*)(const Color&, const Color&)>(ovis::operator*),
                    static_cast<Color (*)(float, const Color&)>(ovis::operator*),
                    static_cast<Color (*)(const Color&, float)>(ovis::operator*));

  /// Divides two colors or a scalar and a color.
  // @function __div
  // @param[type=Color|number] c1
  // @param[type=Color|number] c2
  // @treturn Color
  // @usage local c1 = Color.new(1, 2, 4)
  // local c2 = Color.new(8, 4, 16)
  // local c3 = c1 / c2 -- divides two colors component-wise
  // assert(c3 == Color.new(0.125, 0.5, 0.25))
  // local v4 = c1 / 2 -- divies a color by a scalar
  // assert(v4 == Color.new(0.5, 1, 2, 0.5))
  // -- if you divide a scalar by a color a new color is created
  // -- by dividing the scalar by each component of the color
  // local v5 = 2 / c1
  // assert(v5 == Color.new(2, 1, 0.5, 2))
  color_type[sol::meta_function::division] =
      sol::overload(static_cast<Color (*)(const Color&, const Color&)>(ovis::operator/),
                    static_cast<Color (*)(float, const Color&)>(ovis::operator/),
                    static_cast<Color (*)(const Color&, float)>(ovis::operator/));

  /// Provides the length operator.
  // This returns the number of components in the color.
  // @function __len
  // @treturn number The number of compoenents in the color (4).
  // @usage local c = Color.new()
  // assert(#c == 4)
  color_type[sol::meta_function::length] = [](const Color& color) { return 4; };

  /// Provides string conversion.
  // @function __tostring
  // @treturn string The color components formatted as "(r, g, b, a)"
  // @usage local v = Color.new(1, 0.5, 1)
  // assert(tostring(v) == '(1.0, 0.5, 1.0, 1.0)')
  color_type[sol::meta_function::to_string] = [](const Color& color) { return fmt::format("{}", color); };

  // clang-format on
}
}  // namespace ovis