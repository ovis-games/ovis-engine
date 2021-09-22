#include <ovis/core/vector.hpp>

namespace ovis {

void Vector2::RegisterType(sol::table* module) {
  /// A two-dimensional vector.
  // @classmod ovis.core.Vector2
  // @usage local core = require "ovis.core"
  // local Vector2 = core.Vector2
  // @usage -- Here is a small example that shows how the Vector2 can be used
  // -- in practice by creating a function that computes the
  // -- intersection points of a sphere and a line
  // -- Compute the intersection points between a line and a circle
  // -- @tparam Vector2 p Point on the line
  // -- @tparam Vector2 d Direction of the line
  // -- @tparam Vector2 c Center of the circle
  // -- @tparam number r radius of the circle
  // function intersect_line_circle(p, d, c, r)
  //   local o = p - c
  //   local d_dot_o = Vector2.dot(d, o)
  //   local mag_d = Vector2.length(d)
  //   local delta = d_dot_o^2 - mag_d^2 * (Vector2.length(o)^2 - r^2)
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
  // local p1, p2 = intersect_line_circle(Vector2.new(1, 1),  -- position on line
  //                                     Vector2.new(1, 0),   -- line direction
  //                                     Vector2.new(10, 1),  -- circle center
  //                                     2)                   -- circle radius
  // assert(p1 == Vector2.new(8,1))
  // assert(p2 == Vector2.new(12,1))
  sol::usertype<Vector2> vector2_type = module->new_usertype<Vector2>(
      "Vector2", sol::factories([]() { return Vector2(0, 0); }, [](float x, float y) { return Vector2(x, y); },
                                [](const sol::table& t) {
                                  return Vector2(t.get_or("x", t.get_or(1, 0.0f)), t.get_or("y", t.get_or(2, 0.0f)));
                                }));

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
  // @usage local v = Vector2.new()
  // assert(v.x == 0 and v.y == 0)

  /// Creates a new vector with the given arguments.
  // @function new
  // @param[type=number] x The value for the x component of the vector.
  // @param[type=number] y The value for the y component of the vector.
  // @treturn Vector2
  // @usage local v = Vector2.new(4.0, 3.5)
  // assert(v.x == 4 and v.y == 3.5)

  /// Creates a new vector from a table.
  // @function Vector2.new(
  // @param[type=table] table
  // @treturn Vector
  // @usage -- All the following calls create the same vector:
  // local v1 = Vector2.new(1, 2)
  // local v2 = Vector2.new{1, 2}
  // local v3 = Vector2.new{x = 1, y = 2}
  // assert(v1 == v2)
  // assert(v2 == v3)

  /// Compares two vectors for equality.
  // Be very careful when comparing two vectors as the components are floating point values!
  // @function __eq
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn bool
  // @usage local v1 = Vector2.new(1.0, 1.0)
  // local v2 = Vector2.new(1.0, 1.0)
  // local v3 = Vector2.new(1.0, 0.0)
  // assert(v1 == v2) -- v1 and v2 are equal
  // assert(v1 ~= v3) -- v1 and v3 are not
  vector2_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Vector2&, const Vector2&)>(ovis::operator==);

  /// Adds two vectors.
  // @function __add
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn Vector2
  // @usage local v1 = Vector2.new(1, 2)
  // local v2 = Vector2.new(3, 4)
  // local v3 = v1 + v2
  // assert(v3 == Vector2.new(4, 6))
  vector2_type[sol::meta_function::addition] =
      static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator+);

  /// Negates a vector
  //
  // @function __unm
  // @param[type=Vector2] v
  // @treturn Vector2
  // @usage local v1 = Vector2.new(1, -2)
  // local v2 = -v1
  // assert(v2 == Vector2.new(-1, 2))
  vector2_type[sol::meta_function::unary_minus] = static_cast<Vector2 (*)(const Vector2&)>(ovis::operator-);

  /// Subtracts two vectors.
  // @function __sub
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn Vector2
  // @usage local v1 = Vector2.new(1, 2)
  // local v2 = Vector2.new(3, 4)
  // local v3 = v1 - v2
  // assert(v3 == Vector2.new(-2, -2))
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
  // @usage local v1 = Vector2.new(1, 2)
  // local v2 = Vector2.new(3, 4)
  // local v3 = v1 * v2 -- multiply two vectors component-wise
  // assert(v3 == Vector2.new(3, 8))
  // local v4 = v1 * 2 -- multiply a vector and a scalar
  // assert(v4 == Vector2.new(2, 4))
  // local v5 = 2 * v1 -- you can also multiply from the other side
  // assert(v5 == Vector2.new(2, 4))
  vector2_type[sol::meta_function::multiplication] =
      sol::overload(static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator*),
                    static_cast<Vector2 (*)(float, const Vector2&)>(ovis::operator*),
                    static_cast<Vector2 (*)(const Vector2&, float)>(ovis::operator*));

  /// Divides two vectors or a scalar and a vector.
  // @function __div
  // @param[type=Vector2|number] v1
  // @param[type=Vector2|number] v2
  // @treturn Vector2
  // @usage local v1 = Vector2.new(1, 2)
  // local v2 = Vector2.new(8, 4)
  // local v3 = v1 / v2 -- divides two vectors component-wise
  // assert(v3 == Vector2.new(0.125, 0.5))
  // local v4 = v1 / 2 -- divies a vector by a scalar
  // assert(v4 == Vector2.new(0.5, 1))
  // -- if you divide a scalar by a vector a new vector is created
  // -- by dividing the scalar by each component of the vector
  // local v5 = 2 / v1
  // assert(v5 == Vector2.new(2, 1))
  vector2_type[sol::meta_function::division] =
      sol::overload(static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator/),
                    static_cast<Vector2 (*)(float, const Vector2&)>(ovis::operator/),
                    static_cast<Vector2 (*)(const Vector2&, float)>(ovis::operator/));

  /// Provides the length operator.
  // This returns the number of components in the vector, not its magnitude. For that use the length() function.
  // @see ovis.math.length
  // @function __len
  // @treturn number The number of compoenents in the vector (2).
  // @usage local v = Vector2.new(1.0, 2.0)
  // assert(#v == 2)
  vector2_type[sol::meta_function::length] = [](const Vector2& vector) { return 2; };

  /// Provides string conversion.
  // @function __tostring
  // @treturn string The vector components formatted as "(x, y)"
  // @usage local v = Vector2.new(1.0, 2.0)
  // assert(tostring(v) == '(1.0, 2.0)')
  vector2_type[sol::meta_function::to_string] = [](const Vector2& vector) { return fmt::format("{}", vector); };

  /// Returns a vector with the minimum component-wise values of the inputs.
  // @function min
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn Vector2
  // @usage assert(Vector2.min(Vector2.new(0, 3), Vector2.new(1, 2)) == Vector2.new(0, 2))
  vector2_type["min"] = &ovis::min<Vector2>;

  /// Returns a vector with the maximum component-wise values of the inputs.
  // @function max
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn Vector2
  // @usage assert(Vector2.max(Vector2.new(0, 3), Vector2.new(1, 2)) == Vector2.new(1, 3))
  vector2_type["max"] = &ovis::max<Vector2>;

  /// Clamps the components of the vector to the specified range.
  // @function clamp
  // @param[type=Vector2] v
  // @param[type=Vector2] min
  // @param[type=Vector2] max
  // @treturn Vector2 The vector with the components clamped to the range [min, max]
  // @usage assert(Vector2.clamp(Vector2.new(-1, 2), Vector2.ZERO, Vector2.ONE) == Vector2.new(0, 1))
  vector2_type["clamp"] = &ovis::clamp<Vector2>;

  /// Calculates the squared length of a vector.
  // This is faster than computing the actual length of the vector.
  // @function length_squared
  // @param[type=Vector2] v
  // @treturn number The squared length of the vector
  // @usage assert(Vector2.length_squared(Vector2.new(5, 5)) == 50)
  vector2_type["length_squared"] = &ovis::SquaredLength<Vector2>;

  /// Calculates the length of a vector.
  // @function length
  // @param[type=Vector2] v
  // @treturn number The length of the vector
  // @usage assert(Vector2.length(Vector2.new(5, 5)) > 7.0710)
  // assert(Vector2.length(Vector2.new(5, 5)) < 7.0711)
  vector2_type["length"] = &Length<Vector2>;

  /// Returns the normalized vector.
  // @function normalize
  // @param[type=Vector2] v
  // @treturn Vector2 Returns a vector with the same direction but with a length of 1
  // @usage assert(Vector2.normalize(Vector2.new(5, 0)) == Vector2.new(1, 0))
  vector2_type["normalize"] = &Normalize<Vector2>;

  /// Calculates the dot product between two vectors.
  // @function dot
  // @see Vector2.cross
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn number
  // @usage local v1 = Vector2.new(1, 2)
  // local v2 = Vector2.new(4, 5)
  // assert(Vector2.dot(v1, v2) == 14)
  vector2_type["dot"] = &Dot<Vector2>;

  // clang-format on
}

void Vector2::RegisterType(ScriptContext* context) {
  // vector2_type["x"] = &Vector2::x;
  // vector2_type["y"] = &Vector2::y;
  // vector2_type["z"] = &Vector2::z;
  // vector2_type["ZERO"] = sol::property(Vector2::Zero);
  // vector2_type["ONE"] = sol::property(Vector2::One);
  // vector2_type["POSITIVE_X"] = sol::property(Vector2::PositiveX);
  // vector2_type["NEGATIVE_X"] = sol::property(Vector2::NegativeX);
  // vector2_type["POSITIVE_Y"] = sol::property(Vector2::PositiveY);
  // vector2_type["NEGATIVE_Y"] = sol::property(Vector2::NegativeY);
  // vector2_type["POSITIVE_Z"] = sol::property(Vector2::PositiveZ);
  // vector2_type["NEGATIVE_Z"] = sol::property(Vector2::NegativeZ);

  context->RegisterType<Vector2>("Vector2");
  context->RegisterConstructor<Vector2, float, float>("Create Vector2", {"x", "y"}, "Vector2");
  // vector2_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Vector2&, const Vector2&)>(ovis::operator==);
  context->RegisterFunction<static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator+)>("vector2_add", {"first vector", "second vector"}, {"vector"});
  context->RegisterFunction<static_cast<Vector2 (*)(const Vector2&)>(ovis::operator-)>("vector2_negate", {"vector"}, {"vector"});
  context->RegisterFunction<static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator-)>("vector2_subtract", {"first vector", "second vector"}, {"vector"});
  context->RegisterFunction<static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator*)>("vector2_multiply", {"first vector", "second vector"}, {"vector"});
  context->RegisterFunction<static_cast<Vector2 (*)(const Vector2&, float)>(ovis::operator*)>("vector2_multiply_scalar", {"vector", "scalar"}, {"vector"});
  const auto vector2_to_string = [](const Vector2& vector) { return fmt::format("{}", vector); };
  context->RegisterFunction<static_cast<std::string(*)(const Vector2&)>(vector2_to_string)>("vector2_to_string", {"vector"}, {"string"});
  context->RegisterFunction<&ovis::min<Vector2>>("vector2_min", {"first vector", "second vector"}, {"minimum"});
  context->RegisterFunction<&ovis::max<Vector2>>("vector2_max", {"first vector", "second vector"}, {"maximum"});
  context->RegisterFunction<&ovis::clamp<Vector2>>("vector2_clamp", {"vector", "min", "max"}, {"clamped vector"});
  context->RegisterFunction<&SquaredLength<Vector2>>("vector2_squared_length", {"vector"}, {"squared length"});
  context->RegisterFunction<&Length<Vector2>>("vector2_length", {"vector"}, {"length"});
  context->RegisterFunction<&Normalize<Vector2>>("vector2_normalize", {"vector"}, {"normalized vector"});
  context->RegisterFunction<&Dot<Vector2>>("vector2_dot", {"first vector", "second vector"}, {"dot product"});
}

}  // namespace ovis
