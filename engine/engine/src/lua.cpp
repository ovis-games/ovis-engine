#include <middleclass.hpp>

#include <ovis/math/vector.hpp>
#include <ovis/engine/input.hpp>
#include <ovis/engine/lua.hpp>
#include <ovis/engine/module.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/scene_controller.hpp>
#include <ovis/engine/script_scene_controller.hpp>

namespace ovis {

sol::state Lua::state;
EventHandler<void(const std::string&)> Lua::on_error;

void RegisterVector2(sol::state& state);
void RegisterVector3(sol::state& state);

void Lua::SetupEnvironment() {
  state.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::math, sol::lib::table);
  state.require_script("class", middleclass::SOURCE);

  state["log_error"] = [](const std::string& message) { LogE("{}", message); };
  state["log_warning"] = [](const std::string& message) { LogW("{}", message); };
  state["log_info"] = [](const std::string& message) { LogI("{}", message); };
  state["log_debug"] = [](const std::string& message) { LogD("{}", message); };
  state["log_verbose"] = [](const std::string& message) { LogV("{}", message); };

  state["OvisErrorHandler"] = [](const std::string& message) { on_error.Invoke(message); };
  sol::protected_function::set_default_handler(state["OvisErrorHandler"]);

  RegisterVector2(state);
  RegisterVector3(state);

  // auto Vector3_factories = sol::factories([](sol::table table) { return Vector3(table[1], table[2], table[3]); });
  // sol::usertype<Vector3> Vector3_type = state.new_usertype<Vector3>(
  //     "Vector3", sol::constructors<Vector3(), Vector3(float, float, float)>(), sol::call_constructor,
  //     sol::constructors<Vector3(), Vector3(float, float, float)>(), sol::meta_function::construct,
  //     Vector3_factories);
  // Vector3_type["x"] = &Vector3::x;
  // Vector3_type["y"] = &Vector3::y;
  // Vector3_type["z"] = &Vector3::z;
  // Vector3_type["__tostring"] = [](const Vector3& vector) { return fmt::format("{}", vector); };

  // Vector3_type[sol::meta_function::multiplication] = [](const Vector3& vector, float scalar) {
  //   return vector * scalar;
  // };
  // Vector3_type[sol::meta_function::unary_minus] = [](const Vector3& vector) { return -vector; };

  // sol::usertype<vector4> vector4_type =
  //     state.new_usertype<vector4>("Vector4", sol::constructors<vector4(), vector4(float, float, float, float)>());
  // vector4_type["x"] = &vector4::x;
  // vector4_type["y"] = &vector4::y;
  // vector4_type["z"] = &vector4::z;
  // vector4_type["w"] = &vector4::w;
  // vector4_type["__tostring"] = [](const vector4& vector) { return fmt::format("{}", vector); };

  // sol::usertype<Scene> scene_type = state.new_usertype<Scene>("Scene");
  // scene_type["CreateObject"] = static_cast<SceneObject* (Scene::*)(const std::string&)>(&Scene::CreateObject);
  // scene_type["DeleteObject"] = &Scene::DeleteObject;
  // scene_type["GetObject"] = &Scene::GetObject;

  Module::RegisterToLua();
  SceneObject::RegisterToLua();
  SceneController::RegisterToLua();
  ScriptSceneController::RegisterToLua();
}

sol::protected_function_result Lua::Execute(const std::string& code, const std::string& chunk_name) {
  return state.do_string(code, "=" + chunk_name);
}

sol::protected_function_result Lua::Execute(const std::string& code, const std::string& chunk_name,
                                            std::function<void(const std::string&)> error_handler) {
  auto subscription = on_error.Subscribe(error_handler);

  auto result = Execute(code, chunk_name);
  if (!result.valid()) {
    error_handler(result);
  }

  subscription.Unsubscribe();

  return result;
}

void RegisterVector2(sol::state& state) {
  // clang-format off

  /// a test module
  // @module math

  /// A two-dimensional vector.
  // @type Vector2
  sol::usertype<Vector2> vector2_type =
      state.new_usertype<Vector2>("Vector2", sol::constructors<Vector2(), Vector2(float, float)>());

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
  // The x and y component of the vector will be set to 0.
  // @function new
  // @treturn Vector2
  // @usage local v = Vector2:new()
  // assert(v.x == 0 and v.y == 0)

  /// Creates a new vector with the given arguments.
  // @function new
  // @param[type=number] x The value for the x component of the vector.
  // @param[type=number] y The value for the y component of the vector.
  // @treturn Vector2
  // @usage local v = Vector2:new(4.0, 3.5)
  // assert(v.x == 4 and v.y == 3.5)

  /// Compares two vectors for equality.
  //
  // Be very careful when comparing two vectors as the components are floating point values!
  //
  // @function __eq
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn bool
  // @usage local v1 = Vector2:new(1.0, 1.0)
  // local v2 = Vector2:new(1.0, 1.0)
  // local v3 = Vector2:new(1.0, 0.0)
  // assert(v1 == v2)       -- v1 and v2 are equal
  // assert(not (v1 == v3)) -- v1 and v3 are not
  vector2_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Vector2&, const Vector2&)>(ovis::operator==);

  /// Compares two vectors for inequality.
  //
  // Be very careful when comparing two vectors as the components are floating point values!
  //
  // @function __eq
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn bool
  // @usage local v1 = Vector2:new(1.0, 1.0)
  // local v2 = Vector2:new(1.0, 0.0)
  // local v3 = Vector2:new(1.0, 1.0)
  // assert(v1 ~= v2)       -- v1 and v2 are not equal
  // assert(not (v1 ~= v3)) -- v1 and v3 are
  vector2_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Vector2&, const Vector2&)>(ovis::operator==);

  /// Adds two vectors.
  // @function __add
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn Vector2
  // @usage local v1 = Vector2:new(1, 2)
  // local v2 = Vector2:new(3, 4)
  // local v3 = v1 + v2
  // assert(v3 == Vector2:new(4, 6))
  vector2_type[sol::meta_function::addition] =
      static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator+);

  /// Negates a vector
  //
  // @function __unm
  // @param[type=Vector2] v
  // @treturn Vector2
  // @usage local v1 = Vector2:new(1, -2)
  // local v2 = -v1
  // assert(v2 == Vector2:new(-1, 2))
  vector2_type[sol::meta_function::unary_minus] = static_cast<Vector2 (*)(const Vector2&)>(ovis::operator-);

  /// Subtracts two vectors.
  // @function __sub
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn Vector2
  // @usage local v1 = Vector2:new(1, 2)
  // local v2 = Vector2:new(3, 4)
  // local v3 = v1 - v2
  // assert(v3 == Vector2:new(-2, -2))
  vector2_type[sol::meta_function::subtraction] =
      static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator-);

  /// Multiplies two vectors or a scalar and a vector.
  // Be careful, this is a component-wise multiplication. If you want to calculate the dot product use the dot()
  // function.
  // @see dot
  // @function __mul
  // @param[type=Vector2|number] v1
  // @param[type=Vector2|number] v2
  // @treturn Vector2
  // @usage local v1 = Vector2:new(1, 2)
  // local v2 = Vector2:new(3, 4)
  // local v3 = v1 * v2 -- multiply two vectors component-wise
  // assert(v3 == Vector2:new(3, 8))
  // local v4 = v1 * 2 -- multiply a vector and a scalar
  // assert(v4 == Vector2:new(2, 4))
  // local v5 = 2 * v1 -- you can also multiply from the other side
  // assert(v5 == Vector2:new(2, 4))
  vector2_type[sol::meta_function::multiplication] =
      sol::overload(static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator*),
                    static_cast<Vector2 (*)(float, const Vector2&)>(ovis::operator*),
                    static_cast<Vector2 (*)(const Vector2&, float)>(ovis::operator*));

  /// Divides two vectors or a scalar and a vector.
  // @function __div
  // @param[type=Vector2|number] v1
  // @param[type=Vector2|number] v2
  // @treturn Vector2
  // @usage local v1 = Vector2:new(1, 2)
  // local v2 = Vector2:new(8, 4)
  // local v3 = v1 / v2 -- divides two vectors component-wise
  // assert(v3 == Vector2:new(0.125, 0.5))
  // local v4 = v1 / 2 -- divies a vector by a scalar
  // assert(v4 == Vector2:new(0.5, 1))
  // -- if you divide a scalar by a vector a new vector is created
  // -- by dividing the scalar by each component of the vector
  // local v5 = 2 / v1
  // assert(v5 == Vector2:new(2, 1))
  vector2_type[sol::meta_function::division] =
      sol::overload(static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(ovis::operator/),
                    static_cast<Vector2 (*)(float, const Vector2&)>(ovis::operator/),
                    static_cast<Vector2 (*)(const Vector2&, float)>(ovis::operator/));

  /// Provides the length operator.
  // This returns the number of components in the vector, not its magnitude. For that use the length() function.
  // @see length
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
  // @treturn Vector2 Returns the equivalent of Vector2:new(min(v1.x, v2.x), min(v1.y, v2.y))
  // @usage local v1 = Vector2:new(1.0, -1.0)
  // local v2 = Vector2:new(0.5, 2.0)
  // local min1 = Vector2.min(v1, v2) -- Can be used like this
  // local min2 = v1:min(v2)          -- or like this
  // assert(min1.x == 0.5 and min1.y == -1)
  // assert(min1 == min2)
  vector2_type["min"] = &ovis::min<Vector2>;

  /// Returns a vector with the maximum component-wise values of the inputs.
  // @function max
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn Vector2 Returns the equivalent of Vector2:new(max(v1.x, v2.x), max(v1.y, v2.y))
  // @usage local v1 = Vector2:new(1.0, -1.0)
  // local v2 = Vector2:new(0.5, 2.0)
  // local max1 = Vector2.max(v1, v2) -- Can be used like this
  // local max2 = v1:max(v2)          -- or like this
  // assert(max1.x == 1 and max1.y == 2)
  // assert(max1 == max2)
  vector2_type["max"] = &ovis::max<Vector2>;

  /// Clamps the components of the vector to the specified range.
  // @function clamp
  // @param[type=Vector2] v
  // @param[type=Vector2] min
  // @param[type=Vector2] max
  // @treturn Vector2 The vector with the components clamped to the range [min, max]
  // @usage local v = Vector2:new(1.0, -1.0)
  // local min = Vector2.ZERO
  // local max = Vector2:new(0.5, 0.5)
  // local clamped_vector = Vector2.clamp(v, min, max)
  // assert(clamped_vector.x == 0.5)
  // assert(clamped_vector.y == 0)
  vector2_type["clamp"] = &ovis::clamp<Vector2>;

  /// Calculates the squared length of a vector.
  // This is faster than computing the actual length of the vector.
  // @function squared_length
  // @param[type=Vector2] v
  // @treturn number The squared length of the vector
  // @usage local v = Vector2:new(5.0, 5.0)
  // local squared_length = Vector2.squared_length(v)
  // assert(squared_length == 50)
  vector2_type["squared_length"] = &SquaredLength<Vector2>;

  /// Calculates the length of a vector.
  // @function length
  // @param[type=Vector2] v
  // @treturn number The length of the vector
  // @usage local v = Vector2:new(5.0, 5.0)
  // local length = Vector2.length(v)
  // assert(length > 7.07106 and length < 7.07107)
  vector2_type["length"] = &Length<Vector2>;

  /// Returns the normalized vector.
  // @function normalize
  // @param[type=Vector2] v
  // @treturn Vector2 Returns a vector with the same direction but with a length of 1
  // @usage local v = Vector2:new(5.0, 0.0)
  // local v_norm = Vector2.normalize(v)
  // assert(v_norm == Vector2:new(1.0, 0))
  vector2_type["normalize"] = &Normalize<Vector2>;

  /// Calculates the dot product between two vectors.
  // @function dot
  // @param[type=Vector2] v1
  // @param[type=Vector2] v2
  // @treturn number Returns the result of v1.x*v2.x + v1.y*v2.y
  // @usage local v1 = Vector2:new(1.0, 0.0)
  // local v2 = Vector2:new(0.5, 2.0)
  // local d1 = Vector2.dot(v1, v2) -- Can be used like this
  // local d2 = v1:dot(v2)          -- or like this
  // -- In this case they both return 0.5
  // assert(d1 == 0.5)
  // assert(d2 == 0.5)
  vector2_type["dot"] = &Dot<Vector2>;
  
  // clang-format on
}

void RegisterVector3(sol::state& state) {
  // clang-format off

  /// A two-dimensional vector.
  // @type Vector3
  sol::usertype<Vector3> vector3_type =
      state.new_usertype<Vector3>("Vector3", sol::constructors<Vector3(), Vector3(float, float, float)>());

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
  // @function new
  // @treturn Vector3
  // @usage local v = Vector3:new()
  // assert(v.x == 0 and v.y == 0 and v.z == 0)

  /// Creates a new vector with the given arguments.
  // @function new
  // @param[type=number] x The value for the x component of the vector.
  // @param[type=number] y The value for the y component of the vector.
  // @param[type=number] z The value for the y component of the vector.
  // @treturn Vector3
  // @usage local v = Vector3:new(4.0, 3.5, 5)
  // assert(v.x == 4 and v.y == 3.5 and v.z == 5)

  /// Compares two vectors for equality.
  //
  // Be very careful when comparing two vectors as the components are floating point values!
  //
  // @function __eq
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn bool
  // @usage local v1 = Vector3:new(1.0, 2.0, 3.0)
  // local v2 = Vector3:new(1.0, 2.0, 3.0)
  // local v3 = Vector3:new(1.0, 0.0, 3.0)
  // assert(v1 == v2)       -- v1 and v2 are equal
  // assert(not (v1 == v3)) -- v1 and v3 are not
  vector3_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Vector3&, const Vector3&)>(ovis::operator==);

  /// Compares two vectors for inequality.
  //
  // Be very careful when comparing two vectors as the components are floating point values!
  //
  // @function __eq
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn bool
  // @usage local v1 = Vector3:new(1.0, 2.0, 3.0)
  // local v2 = Vector3:new(1.0, 2.0, 3.0)
  // local v3 = Vector3:new(1.0, 0.0, 3.0)
  // assert(v1 ~= v2)       -- v1 and v2 are not equal
  // assert(not (v1 ~= v3)) -- v1 and v3 are
  vector3_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Vector3&, const Vector3&)>(ovis::operator==);

  /// Adds two vectors.
  // @function __add
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn Vector3
  // @usage local v1 = Vector3:new(1, 2, 3)
  // local v2 = Vector3:new(4, 5, 6)
  // local v3 = v1 + v2
  // assert(v3 == Vector3:new(5, 7, 9))
  vector3_type[sol::meta_function::addition] =
      static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator+);

  /// Negates a vector
  //
  // @function __unm
  // @param[type=Vector3] v
  // @treturn Vector3
  // @usage local v1 = Vector3:new(1, -2, 3)
  // local v2 = -v1
  // assert(v2 == Vector3:new(-1, 2, -3))
  vector3_type[sol::meta_function::unary_minus] = static_cast<Vector3 (*)(const Vector3&)>(ovis::operator-);

  /// Subtracts two vectors.
  // @function __sub
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn Vector3
  // @usage local v1 = Vector3:new(1, 2, 3)
  // local v2 = Vector3:new(4, 5, 6)
  // local v3 = v1 - v2
  // assert(v3 == Vector3:new(-3, -3, -3))
  vector3_type[sol::meta_function::subtraction] =
      static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator-);

  /// Multiplies two vectors or a scalar and a vector.
  // Be careful, this is a component-wise multiplication. If you want to calculate the dot product use the dot()
  // function.
  // @see dot
  // @function __mul
  // @param[type=Vector3|number] v1
  // @param[type=Vector3|number] v2
  // @treturn Vector3
  // @usage local v1 = Vector3:new(1, 2, 3)
  // local v2 = Vector3:new(4, 5, 6)
  // local v3 = v1 * v2 -- multiply two vectors component-wise
  // assert(v3 == Vector3:new(4, 10, 18))
  // local v4 = v1 * 2 -- multiply a vector and a scalar
  // assert(v4 == Vector3:new(2, 4, 6))
  // local v5 = 2 * v1 -- you can also multiply from the other side
  // assert(v5 == Vector3:new(2, 4, 6))
  vector3_type[sol::meta_function::multiplication] =
      sol::overload(static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator*),
                    static_cast<Vector3 (*)(float, const Vector3&)>(ovis::operator*),
                    static_cast<Vector3 (*)(const Vector3&, float)>(ovis::operator*));

  /// Divides two vectors or a scalar and a vector.
  // @function __div
  // @param[type=Vector3|number] v1
  // @param[type=Vector3|number] v2
  // @treturn Vector3
  // @usage local v1 = Vector3:new(1, 2, 4)
  // local v2 = Vector3:new(8, 4, 16)
  // local v3 = v1 / v2 -- divides two vectors component-wise
  // assert(v3 == Vector3:new(0.125, 0.5, 0.25))
  // local v4 = v1 / 2 -- divies a vector by a scalar
  // assert(v4 == Vector3:new(0.5, 1, 2))
  // -- if you divide a scalar by a vector a new vector is created
  // -- by dividing the scalar by each component of the vector
  // local v5 = 2 / v1
  // assert(v5 == Vector3:new(2, 1, 0.5))
  vector3_type[sol::meta_function::division] =
      sol::overload(static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(ovis::operator/),
                    static_cast<Vector3 (*)(float, const Vector3&)>(ovis::operator/),
                    static_cast<Vector3 (*)(const Vector3&, float)>(ovis::operator/));

  /// Provides the length operator.
  // This returns the number of components in the vector, not its magnitude. For that use the length() function.
  // @see length
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
  // @treturn Vector3 Returns the equivalent of Vector3:new(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z))
  // @usage local v1 = Vector3:new(1, 4, 2)
  // local v2 = Vector3:new(3, 2, 1)
  // local min1 = Vector3.min(v1, v2) -- Can be used like this
  // local min2 = v1:min(v2)          -- or like this
  // assert(min1 == Vecto3:new(1, 2, 1))
  // assert(min1 == min2)
  vector3_type["min"] = &ovis::min<Vector3>;

  /// Returns a vector with the maximum component-wise values of the inputs.
  // @function max
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn Vector3 Returns the equivalent of Vector3:new(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z))
  // @usage local v1 = Vector3:new(1, 4, 2)
  // local v2 = Vector3:new(3, 2, 1)
  // local max1 = Vector3.max(v1, v2) -- Can be used like this
  // local max2 = v1:max(v2)          -- or like this
  // assert(max1 == Vector3:new(3, 4, 2))
  // assert(max1 == max2)
  vector3_type["max"] = &ovis::max<Vector3>;

  /// Clamps the components of the vector to the specified range.
  // @function clamp
  // @param[type=Vector3] v
  // @param[type=Vector3] min
  // @param[type=Vector3] max
  // @treturn Vector3 The vector with the components clamped to the range [min, max]
  // @usage local v = Vector3:new(1.0, -1.0)
  // local min = Vector3.ZERO
  // local max = Vector3:new(0.5, 0.5)
  // local clamped_vector = Vector3.clamp(v, min, max)
  // assert(clamped_vector.x == 0.5)
  // assert(clamped_vector.y == 0)
  vector3_type["clamp"] = &ovis::clamp<Vector3>;

  /// Calculates the squared length of a vector.
  // This is faster than computing the actual length of the vector.
  // @function squared_length
  // @param[type=Vector3] v
  // @treturn number The squared length of the vector
  // @usage local v = Vector3:new(5.0, 5.0)
  // local squared_length = Vector3.squared_length(v)
  // assert(squared_length == 50)
  vector3_type["squared_length"] = &SquaredLength<Vector3>;

  /// Calculates the length of a vector.
  // @function length
  // @param[type=Vector3] v
  // @treturn number The length of the vector
  // @usage local v = Vector3:new(5.0, 5.0)
  // local length = Vector3.length(v)
  // assert(length > 7.07106 and length < 7.07107)
  vector3_type["length"] = &Length<Vector3>;

  /// Returns the normalized vector.
  // @function normalize
  // @param[type=Vector3] v
  // @treturn Vector3 Returns a vector with the same direction but with a length of 1
  // @usage local v = Vector3:new(5.0, 0.0)
  // local v_norm = Vector3.normalize(v)
  // assert(v_norm == Vector3:new(1.0, 0))
  vector3_type["normalize"] = &Normalize<Vector3>;

  /// Calculates the dot product between two vectors.
  // @function dot
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn number Returns the result of v1.x*v2.x + v1.y*v2.y
  // @usage local v1 = Vector3:new(1.0, 0.0)
  // local v2 = Vector3:new(0.5, 2.0)
  // local d1 = Vector3.dot(v1, v2) -- Can be used like this
  // local d2 = v1:dot(v2)          -- or like this
  // -- In this case they both return 0.5
  // assert(d1 == 0.5)
  // assert(d2 == 0.5)
  vector3_type["dot"] = &Dot<Vector3>;
  
  // clang-format on
}

}  // namespace ovis
