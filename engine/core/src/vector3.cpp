#include <ovis/core/vector.hpp>

#include <ovis/core/virtual_machine.hpp>

namespace ovis {

void Vector3::RegisterType(sol::table* module) {
  // clang-format off

  /// A three-dimensional vector.
  // @classmod ovis.core.Vector3
  // @usage local core = require "ovis.core"
  // local Vector3 = core.Vector3
  // @usage -- Here is a small example that shows how the Vector3 can be used
  // -- in practice by creating a function that computes the
  // -- intersection points of a ray and a plane.
  // 
  // -- Compute the intersection points between a ray and a plane.
  // -- @tparam Vector3 o Origin of the ray
  // -- @tparam Vector3 d Direction of the ray
  // -- @tparam Vector3 p Point on the plane
  // -- @tparam Vector3 n Normal of the plane
  // function intersect_ray_plane(o, d, p, n)
  //   local denominator = Vector3.dot(d, n)
  //   if math.abs(denominator) < 0.000001 then -- use your favourite epsilon here
  //     return nil, 'No intersection found'
  //   end
  //   local t = Vector3.dot(p - o, n) / denominator
  //   if t >= 0 then
  //     return o + t * d
  //   else
  //     return nil, 'No intersection found'
  //   end
  // end
  //
  // local p = intersect_ray_plane(Vector3.new(10, 10, 10), -- origin of the ray
  //                               Vector3.NEGATIVE_Y,  -- ray direction
  //                               Vector3.new(0, 2, 0),    -- point on the plane
  //                               Vector3.POSITIVE_Y)  -- plane normal
  // assert(p == Vector3.new(10, 2, 10))
  sol::usertype<Vector3> vector3_type = module->new_usertype<Vector3>(
    "Vector3", sol::factories(
      []() { return Vector3(0, 0, 0); },
      [](float x, float y, float z) { return Vector3(x, y, z); },
      [](const sol::table& t) {
        return Vector3(
          t.get_or("x", t.get_or(1, 0.0f)),
          t.get_or("y", t.get_or(2, 0.0f)),
          t.get_or("z", t.get_or(3, 0.0f))
        );
      }
    )
  );

  /// The x component of the vector.
  // @field[type=number] x
  vector3_type["x"] = &Vector3::x;

  /// The y component of the vector.
  // @field[type=number] y
  vector3_type["y"] = &Vector3::y;

  /// The z component of the vector.
  // @field[type=number] z
  vector3_type["z"] = &Vector3::z;

  /// A 3D vector with all components set to zero.
  // @field[type=Vector3] ZERO
  // @usage local v = Vector3.ZERO
  // assert(v.x == 0 and v.y == 0 and v.z == 0)
  vector3_type["ZERO"] = sol::property(Vector3::Zero);

  /// A 3D vector with all components set to one.
  // @field[type=Vector3] ONE
  // @usage local v = Vector3.ONE
  // assert(v.x == 1 and v.y == 1 and v.z == 1)
  vector3_type["ONE"] = sol::property(Vector3::One);

  /// A 3D vector with the x component set to one.
  // @field[type=Vector3] POSITIVE_X
  // @usage local v = Vector3.POSITIVE_X
  // assert(v.x == 1 and v.y == 0 and v.z == 0)
  vector3_type["POSITIVE_X"] = sol::property(Vector3::PositiveX);

  /// A 3D vector with the x component set to minus one.
  // @field[type=Vector3] NEGATIVE_X
  // @usage local v = Vector3.NEGATIVE_X
  // assert(v.x == -1 and v.y == 0 and v.z == 0)
  vector3_type["NEGATIVE_X"] = sol::property(Vector3::NegativeX);

  /// A 3D vector with the y component set to one.
  // @field[type=Vector3] POSITIVE_Y
  // @usage local v = Vector3.POSITIVE_Y
  // assert(v.x == 0 and v.y == 1 and v.z == 0)
  vector3_type["POSITIVE_Y"] = sol::property(Vector3::PositiveY);

  /// A 3D vector with the y component set to minus one.
  // @field[type=Vector3] NEGATIVE_Y
  // @usage local v = Vector3.NEGATIVE_Y
  // assert(v.x == 0 and v.y == -1 and v.z == 0)
  vector3_type["NEGATIVE_Y"] = sol::property(Vector3::NegativeY);

  /// A 3D vector with the z component set to one.
  // @field[type=Vector3] POSITIVE_Z
  // @usage local v = Vector3.POSITIVE_Z
  // assert(v.x == 0 and v.y == 0 and v.z == 1)
  vector3_type["POSITIVE_Z"] = sol::property(Vector3::PositiveZ);

  /// A 3D vector with the z component set to minus one.
  // @field[type=Vector3] NEGATIVE_Z
  // @usage local v = Vector3.NEGATIVE_Z
  // assert(v.x == 0 and v.y == 0 and v.z == -1)
  vector3_type["NEGATIVE_Z"] = sol::property(Vector3::NegativeZ);

  /// Creates a new vector.
  // The x, y and z component of the vector will be set to 0.
  // @function Vector3.new
  // @treturn Vector3
  // @usage local v = Vector3.new()
  // assert(v.x == 0 and v.y == 0 and v.z == 0)

  /// Creates a new vector with the given arguments.
  // @function Vector3.new
  // @param[type=number] x The value for the x component of the vector.
  // @param[type=number] y The value for the y component of the vector.
  // @param[type=number] z The value for the y component of the vector.
  // @treturn Vector3
  // @usage local v = Vector3.new(4.0, 3.5, 5)
  // assert(v.x == 4 and v.y == 3.5 and v.z == 5)

  /// Creates a new vector from a table.
  // @function Vector3.new
  // @param[type=table] table
  // @treturn Vector3
  // @usage -- All the following calls create the same vector:
  // local v1 = Vector3.new(1, 2, 3)
  // local v2 = Vector3.new{1, 2, 3}
  // local v3 = Vector3.new{x = 1, y = 2, z = 3}
  // assert(v1 == v2)
  // assert(v2 == v3)

  /// Compares two vectors for equality.
  //
  // Be very careful when comparing two vectors as the components are floating point values!
  //
  // @function __eq
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn bool
  // @usage local v1 = Vector3.new(1.0, 2.0, 3.0)
  // local v2 = Vector3.new(1.0, 2.0, 3.0)
  // local v3 = Vector3.new(1.0, 0.0, 3.0)
  // assert(v1 == v2) -- v1 and v2 are equal
  // assert(v1 ~= v3) -- v1 and v3 are not
  vector3_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Vector3&, const Vector3&)>(ovis::operator==);

  /// Adds two vectors.
  // @function __add
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn Vector3
  // @usage local v1 = Vector3.new(1, 2, 3)
  // local v2 = Vector3.new(4, 5, 6)
  // local v3 = v1 + v2
  // assert(v3 == Vector3.new(5, 7, 9))
  vector3_type[sol::meta_function::addition] =
      static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator+);

  /// Negates a vector
  //
  // @function __unm
  // @param[type=Vector3] v
  // @treturn Vector3
  // @usage local v1 = Vector3.new(1, -2, 3)
  // local v2 = -v1
  // assert(v2 == Vector3.new(-1, 2, -3))
  vector3_type[sol::meta_function::unary_minus] = static_cast<Vector3 (*)(const Vector3&)>(ovis::operator-);

  /// Subtracts two vectors.
  // @function __sub
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn Vector3
  // @usage local v1 = Vector3.new(1, 2, 3)
  // local v2 = Vector3.new(4, 5, 6)
  // local v3 = v1 - v2
  // assert(v3 == Vector3.new(-3, -3, -3))
  vector3_type[sol::meta_function::subtraction] =
      static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator-);

  /// Multiplies two vectors or a scalar and a vector.
  // Be careful, this is a component-wise multiplication. If you want to calculate the dot product use @{dot}.
  // @see Vector3.dot
  // @function __mul
  // @param[type=Vector3|number] v1
  // @param[type=Vector3|number] v2
  // @treturn Vector3
  // @usage local v1 = Vector3.new(1, 2, 3)
  // local v2 = Vector3.new(4, 5, 6)
  // local v3 = v1 * v2 -- multiply two vectors component-wise
  // assert(v3 == Vector3.new(4, 10, 18))
  // local v4 = v1 * 2 -- multiply a vector and a scalar
  // assert(v4 == Vector3.new(2, 4, 6))
  // local v5 = 2 * v1 -- you can also multiply from the other side
  // assert(v5 == Vector3.new(2, 4, 6))
  vector3_type[sol::meta_function::multiplication] =
      sol::overload(static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator*),
                    static_cast<Vector3 (*)(float, const Vector3&)>(ovis::operator*),
                    static_cast<Vector3 (*)(const Vector3&, float)>(ovis::operator*));

  /// Divides two vectors or a scalar and a vector.
  // @function __div
  // @param[type=Vector3|number] v1
  // @param[type=Vector3|number] v2
  // @treturn Vector3
  // @usage local v1 = Vector3.new(1, 2, 4)
  // local v2 = Vector3.new(8, 4, 16)
  // local v3 = v1 / v2 -- divides two vectors component-wise
  // assert(v3 == Vector3.new(0.125, 0.5, 0.25))
  // local v4 = v1 / 2 -- divies a vector by a scalar
  // assert(v4 == Vector3.new(0.5, 1, 2))
  // -- if you divide a scalar by a vector a new vector is created
  // -- by dividing the scalar by each component of the vector
  // local v5 = 2 / v1
  // assert(v5 == Vector3.new(2, 1, 0.5))
  vector3_type[sol::meta_function::division] =
      sol::overload(static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator/),
                    static_cast<Vector3 (*)(float, const Vector3&)>(ovis::operator/),
                    static_cast<Vector3 (*)(const Vector3&, float)>(ovis::operator/));

  /// Provides the length operator.
  // This returns the number of components in the vector, not its magnitude. For that use the length() function.
  // @see Vector3.length
  // @function __len
  // @treturn number The number of compoenents in the vector (3).
  // @usage local v = Vector3.new()
  // assert(#v == 3)
  vector3_type[sol::meta_function::length] = [](const Vector3& vector) { return 3; };

  /// Provides string conversion.
  // @function __tostring
  // @treturn string The vector components formatted as "(x, y, z)"
  // @usage local v = Vector3.new(1, 2, 3)
  // assert(tostring(v) == '(1.0, 2.0, 3.0)')
  vector3_type[sol::meta_function::to_string] = [](const Vector3& vector) { return fmt::format("{}", vector); };

  /// Returns a vector with the minimum component-wise values of the inputs.
  // @function min
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn Vector3
  // @usage assert(Vector3.min(Vector3.new(0, 3, 2), Vector3.new(1, 2, 1)) == Vector3.new(0, 2, 1))
  vector3_type["min"] = &ovis::min<Vector3>;

  /// Returns a vector with the maximum component-wise values of the inputs.
  // @function max
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn Vector3
  // @usage assert(Vector3.max(Vector3.new(0, 3, 2), Vector3.new(1, 2, 1)) == Vector3.new(1, 3, 2))
  vector3_type["max"] = &ovis::max<Vector3>;

  /// Clamps the components of the vector to the specified range.
  // @function clamp
  // @param[type=Vector3] v
  // @param[type=Vector3] min
  // @param[type=Vector3] max
  // @treturn Vector3 The vector with the components clamped to the range [min, max]
  // @usage assert(Vector3.clamp(Vector3.new(-1, 2, 0.5), Vector3.ZERO, Vector3.ONE) == Vector3.new(0, 1, 0.5))
  vector3_type["clamp"] = &ovis::clamp<Vector3>;

  /// Calculates the squared length of a vector.
  // This is faster than computing the actual length of the vector.
  // @function length_squared
  // @param[type=Vector3] v
  // @treturn number The squared length of the vector
  // @usage assert(Vector3.length_squared(Vector3.new(5, 5, 5)) == 75)
  vector3_type["length_squared"] = &ovis::SquaredLength<Vector3>;

  /// Calculates the length of a vector.
  // @function length
  // @param[type=Vector3] v
  // @treturn number The length of the vector
  // @usage assert(Vector3.length(Vector3.new(5, 5, 5)) > 8.6602)
  // assert(Vector3.length(Vector3.new(5, 5, 5)) < 8.6603)
  vector3_type["length"] = &Length<Vector3>;

  /// Returns the normalized vector.
  // @function normalize
  // @param[type=Vector3] v
  // @treturn Vector3 Returns a vector with the same direction but with a length of 1
  // @usage assert(Vector3.normalize(Vector3.new(5, 0, 0)) == Vector3.new(1, 0, 0))
  vector3_type["normalize"] = &Normalize<Vector3>;

  /// Calculates the dot product between two vectors.
  // The function is overloaded for both, @{Vector2} and @{Vector3}. However, both
  // inputs need to be of the same type.
  // @function dot
  // @see Vector3.cross
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn number
  // @usage local v1 = Vector3.new(1, 2, 3)
  // local v2 = Vector3.new(4, 5, 6)
  // assert(Vector3.dot(v1, v2) == 32)
  vector3_type["dot"] = &Dot<Vector3>;

  /// Calculates the dot product between two vectorss.
  // @function cross
  // @see dot
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn Vector3
  // @usage assert(Vector3.cross(Vector3.POSITIVE_X, Vector3.POSITIVE_Y) == Vector3.POSITIVE_Z)
  vector3_type["cross"] = &Cross;

  // clang-format off
}

std::string VectorToString(Vector3 v) {
  return fmt::format("{}", v);
}

Vector3 LinearInterpolateVector3(Vector3 a, Vector3 b, float t) {
  // TODO: references
  return (1.0f - t) * a + t * b;
}
vm::Value DeserializeVector3(const json& data) {
  Vector3 value = data;
  return value;
}

void Vector3::RegisterType(vm::Module* module) {
  // vector3_type["x"] = &Vector3::x;
  // vector3_type["y"] = &Vector3::y;
  // vector3_type["z"] = &Vector3::z;
  // vector3_type["ZERO"] = sol::property(Vector3::Zero);
  // vector3_type["ONE"] = sol::property(Vector3::One);
  // vector3_type["POSITIVE_X"] = sol::property(Vector3::PositiveX);
  // vector3_type["NEGATIVE_X"] = sol::property(Vector3::NegativeX);
  // vector3_type["POSITIVE_Y"] = sol::property(Vector3::PositiveY);
  // vector3_type["NEGATIVE_Y"] = sol::property(Vector3::NegativeY);
  // vector3_type["POSITIVE_Z"] = sol::property(Vector3::PositiveZ);
  // vector3_type["NEGATIVE_Z"] = sol::property(Vector3::NegativeZ);

  module->RegisterType<Vector3>("Vector3");
  // module->RegisterConstructor<Vector3, float, float, float>("create_vector3", {"x", "y", "z"}, "vector");
  // vector3_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Vector3&, const Vector3&)>(ovis::operator==);
  module->RegisterFunction<static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator+)>("vector3_add", {"first vector", "second vector"}, {"vector"});
  module->RegisterFunction<static_cast<Vector3 (*)(const Vector3&)>(ovis::operator-)>("vector3_negate", {"vector"}, {"vector"});
  module->RegisterFunction<static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator-)>("vector3_subtract", {"first vector", "second vector"}, {"vector"});
  module->RegisterFunction<static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator*)>("vector3_multiply", {"first vector", "second vector"}, {"vector"});
  module->RegisterFunction<static_cast<Vector3 (*)(const Vector3&, float)>(ovis::operator*)>("vector3_multiply_scalar", {"vector", "scalar"}, {"vector"});
  module->RegisterFunction<&VectorToString>("vector3_to_string", {"vector"}, {"string"});
  module->RegisterFunction<&ovis::min<Vector3>>("vector3_min", {"first vector", "second vector"}, {"minimum"});
  module->RegisterFunction<&ovis::max<Vector3>>("vector3_max", {"first vector", "second vector"}, {"maximum"});
  module->RegisterFunction<&ovis::clamp<Vector3>>("vector3_clamp", {"vector", "min", "max"}, {"clamped vector"});
  module->RegisterFunction<&SquaredLength<Vector3>>("vector3_squared_length", {"vector"}, {"squared length"});
  module->RegisterFunction<&Length<Vector3>>("vector3_length", {"vector"}, {"length"});
  module->RegisterFunction<&Normalize<Vector3>>("vector3_normalize", {"vector"}, {"normalized vector"});
  module->RegisterFunction<&Dot<Vector3>>("vector3_dot", {"first vector", "second vector"}, {"dot product"});
  module->RegisterFunction<&Cross>("vector3_cross", {"first vector", "second vector"}, {"cross product"});
}

}
