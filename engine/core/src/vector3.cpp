#include <ovis/math/vector.hpp>

namespace ovis {

void Vector3::RegisterType(sol::table* module) {
  // clang-format off

  /// A three-dimensional vector.
  // @classmod ovis.math.Vector3
  // @usage local math = require "ovis.math"
  // local Vector3 = math.Vector3
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
  //   local denominator = math.dot(d, n)
  //   if math.abs(denominator) < 0.000001 then -- use your favourite epsilon here
  //     return nil, 'No intersection found'
  //   end
  //   local t = math.dot(p - o, n) / denominator
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
  // @see ovis.math.dot
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
  // @see ovis.math.length
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

  // clang-format off
}

}