#include "module_loader.hpp"

#include <sol/sol.hpp>

#include <ovis/math/vector.hpp>

namespace ovis {

int LoadVector2Module(lua_State* l) {
  sol::state_view state(l);

  /// A two-dimensional vector.
  // @classmod ovis.math.Vector2
  // @usage -- Here is a small example that shows how the Vector2 can be used
  // -- in practice by creating a function that computes the
  // -- intersection points of a sphere and a line
  // --
  // -- Compute the intersection points between a line and a circle
  // -- @tparam Vector2 p Point on the line
  // -- @tparam Vector2 d Direction of the line
  // -- @tparam Vector2 c Center of the circle
  // -- @tparam number r radius of the circle
  // function intersect_line_circle(p, d, c, r)
  //   local o = p - c
  //   local d_dot_o = dot(d, o)
  //   local mag_d = length(d)
  //   local delta = d_dot_o^2 - mag_d^2 * (length(o)^2 - r^2)
  //   if delta < 0 then
  //     return nil, 'No intersection found'
  //   elseif delta < 0.00001 then -- use your favourite epsilon here
  //     t = mag_d
  //     return p + t * d
  //   else
  //     local sqrt_delta = math.sqrt(delta)
  //     local t1 = -(d_dot_o + sqrt_delta) / mag_d
  //     local t2 = -(d_dot_o - sqrt_delta) / mag_d
  //     return p + t1 * d, p + t2 * d
  //   end
  // end
  // local p1, p2 = intersect_line_circle(Vector2(1, 1),  -- position on line
  //                                     Vector2(1, 0),  -- line direction
  //                                     Vector2(10, 1), -- circle center
  //                                     2)              -- circle radius
  // assert(p1 == Vector2(8,1))
  // assert(p2 == Vector2(12,1))
  sol::usertype<Vector2> vector2_type = state.new_usertype<Vector2>(
    "Vector2", sol::constructors<Vector2(), Vector2(float, float)>(),
    sol::call_constructor,
    sol::factories(
      []() { return Vector2(0, 0); },
      [](float x, float y) { return Vector2(x, y); },
      [](const sol::table& t) {
        return Vector2(
          t.get_or("x", t.get_or(1, 0.0f)),
          t.get_or("y", t.get_or(2, 0.0f))
        );
      }
    )
  );

  /// The x component of the vector.
  // @field[type=number] x
  vector2_type["x"] = &Vector2::x;

  /// The y component of the vector.
  // @field[type=number] y
  vector2_type["y"] = &Vector2::y;

  /// A 2D vector with all components set to zero.
  // @field[type=Vector2] ZERO
  // @usage local v = Vector2.ZERO
  // assert(v.x == 0 and v.y == 0)
  vector2_type["ZERO"] = sol::property(Vector2::Zero);

  /// A 2D vector with all components set to one.
  // @field[type=Vector2] ONE
  // @usage local v = Vector2.ONE
  // assert(v.x == 1 and v.y == 1)
  vector2_type["ONE"] = sol::property(Vector2::One);

  /// A 2D vector with the x component set to one.
  // @field[type=Vector2] POSITIVE_X
  // @usage local v = Vector2.POSITIVE_X
  // assert(v.x == 1 and v.y == 0)
  vector2_type["POSITIVE_X"] = sol::property(Vector2::PositiveX);

  /// A 2D vector with the x component set to minus one.
  // @field[type=Vector2] NEGATIVE_X
  // @usage local v = Vector2.NEGATIVE_X
  // assert(v.x == -1 and v.y == 0)
  vector2_type["NEGATIVE_X"] = sol::property(Vector2::NegativeX);

  /// A 2D vector with the y component set to one.
  // @field[type=Vector2] POSITIVE_Y
  // @usage local v = Vector2.POSITIVE_Y
  // assert(v.x == 0 and v.y == 1)
  vector2_type["POSITIVE_Y"] = sol::property(Vector2::PositiveY);

  /// A 2D vector with the y component set to minus one.
  // @field[type=Vector2] NEGATIVE_Y
  // @usage local v = Vector2.NEGATIVE_Y
  // assert(v.x == 0 and v.y == -1)
  vector2_type["NEGATIVE_Y"] = sol::property(Vector2::NegativeY);

  /// Creates a new vector.
  // The x and y component of the vector will be set to 0. Consider using the
  // @{__call} syntax as this is a frequently created type.
  // @function new
  // @treturn Vector2
  // @see __call
  // @usage local v = Vector2:new()
  // assert(v.x == 0 and v.y == 0)

  /// Creates a new vector with the given arguments.
  // Consider using the @{__call} syntax as this is a frequently created type.
  // @function new
  // @param[type=number] x The value for the x component of the vector.
  // @param[type=number] y The value for the y component of the vector.
  // @treturn Vector2
  // @see __call
  // @usage local v = Vector2:new(4.0, 3.5)
  // assert(v.x == 4 and v.y == 3.5)

  /// Creates a new vector.
  // Simplifies the syntax for creating vectors.
  // @function __call
  // @treturn Vector3
  // @usage -- All the following calls create the same vector:
  // local v1 = Vector2:new(1, 2)
  // local v2 = Vector2(1, 2)
  // local v3 = Vector2{1, 2}
  // local v4 = Vector2{x = 1, y = 2}
  // assert(v1 == v2)
  // assert(v2 == v3)
  // assert(v3 == v4)

  /// Compares two vectors for equality.
  //
  // Be very careful when comparing two vectors as the components are floating point values!
  //
  // @function __eq
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn bool
  // @usage local v1 = Vector2(1.0, 1.0)
  // local v2 = Vector2(1.0, 1.0)
  // local v3 = Vector2(1.0, 0.0)
  // assert(v1 == v2) -- v1 and v2 are equal
  // assert(v1 ~= v3) -- v1 and v3 are not
  vector2_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Vector2&, const Vector2&)>(ovis::operator==);

  /// Adds two vectors.
  // @function __add
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn Vector2
  // @usage local v1 = Vector2(1, 2)
  // local v2 = Vector2(3, 4)
  // local v3 = v1 + v2
  // assert(v3 == Vector2(4, 6))
  vector2_type[sol::meta_function::addition] =
      static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator+);

  /// Negates a vector
  //
  // @function __unm
  // @param[type=Vector2] v
  // @treturn Vector2
  // @usage local v1 = Vector2(1, -2)
  // local v2 = -v1
  // assert(v2 == Vector2(-1, 2))
  vector2_type[sol::meta_function::unary_minus] = static_cast<Vector2 (*)(const Vector2&)>(ovis::operator-);

  /// Subtracts two vectors.
  // @function __sub
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn Vector2
  // @usage local v1 = Vector2(1, 2)
  // local v2 = Vector2(3, 4)
  // local v3 = v1 - v2
  // assert(v3 == Vector2(-2, -2))
  vector2_type[sol::meta_function::subtraction] =
      static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator-);

  /// Multiplies two vectors or a scalar and a vector.
  // Be careful, this is a component-wise multiplication. If you want to calculate the dot product use the dot()
  // function.
  // @see ovis.math.dot
  // @function __mul
  // @param[type=Vector2|number] v1
  // @param[type=Vector2|number] v2
  // @treturn Vector2
  // @usage local v1 = Vector2(1, 2)
  // local v2 = Vector2(3, 4)
  // local v3 = v1 * v2 -- multiply two vectors component-wise
  // assert(v3 == Vector2(3, 8))
  // local v4 = v1 * 2 -- multiply a vector and a scalar
  // assert(v4 == Vector2(2, 4))
  // local v5 = 2 * v1 -- you can also multiply from the other side
  // assert(v5 == Vector2(2, 4))
  vector2_type[sol::meta_function::multiplication] =
      sol::overload(static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator*),
                    static_cast<Vector2 (*)(float, const Vector2&)>(ovis::operator*),
                    static_cast<Vector2 (*)(const Vector2&, float)>(ovis::operator*));

  /// Divides two vectors or a scalar and a vector.
  // @function __div
  // @param[type=Vector2|number] v1
  // @param[type=Vector2|number] v2
  // @treturn Vector2
  // @usage local v1 = Vector2(1, 2)
  // local v2 = Vector2(8, 4)
  // local v3 = v1 / v2 -- divides two vectors component-wise
  // assert(v3 == Vector2(0.125, 0.5))
  // local v4 = v1 / 2 -- divies a vector by a scalar
  // assert(v4 == Vector2(0.5, 1))
  // -- if you divide a scalar by a vector a new vector is created
  // -- by dividing the scalar by each component of the vector
  // local v5 = 2 / v1
  // assert(v5 == Vector2(2, 1))
  vector2_type[sol::meta_function::division] =
      sol::overload(static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator/),
                    static_cast<Vector2 (*)(float, const Vector2&)>(ovis::operator/),
                    static_cast<Vector2 (*)(const Vector2&, float)>(ovis::operator/));

  /// Provides the length operator.
  // This returns the number of components in the vector, not its magnitude. For that use the length() function.
  // @see ovis.math.length
  // @function __len
  // @treturn number The number of compoenents in the vector (2).
  // @usage local v = Vector2(1.0, 2.0)
  // assert(#v == 2)
  vector2_type[sol::meta_function::length] = [](const Vector2& vector) { return 2; };

  /// Provides string conversion.
  // @function __tostring
  // @treturn string The vector components formatted as "(x, y)"
  // @usage local v = Vector2(1.0, 2.0)
  // assert(tostring(v) == '(1.0, 2.0)')
  vector2_type[sol::meta_function::to_string] = [](const Vector2& vector) { return fmt::format("{}", vector); };
  
  // clang-format on
  return vector2_type.push();
}

}