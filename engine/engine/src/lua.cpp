#include <middleclass.hpp>

#include <ovis/math/vector.hpp>
#include <ovis/math/color.hpp>
#include <ovis/math/lua_modules/register_modules.hpp>
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
// void RegisterVector3(sol::state& state);
void RegisterColor(sol::state& state);

int foo(lua_State* l) {
  sol::state_view state(l);
  sol::table t = state.create_table();

  t["foo"] = 1;

  return t.push();
}

void Lua::SetupEnvironment() {
  state.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::package);
  state.require_script("class", middleclass::SOURCE);

  state["log_error"] = [](const std::string& message) { LogE("{}", message); };
  state["log_warning"] = [](const std::string& message) { LogW("{}", message); };
  state["log_info"] = [](const std::string& message) { LogI("{}", message); };
  state["log_debug"] = [](const std::string& message) { LogD("{}", message); };
  state["log_verbose"] = [](const std::string& message) { LogV("{}", message); };

  state["OvisErrorHandler"] = [](const std::string& message) { on_error.Invoke(message); };
  sol::protected_function::set_default_handler(state["OvisErrorHandler"]);

  RegisterMathLuaModules(state.lua_state());

  // RegisterVector2(state);
  // RegisterVector3(state);
  // RegisterColor(state);

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

// clang-format off

/// This module contains math related types and function.
// The types defined in this module are designed to behave as close to built-in
// types as possible. E.g., they have all necessary basic math operators
// defined in a meaningful way.
// @module ovis.math
// @usage -- require('ovis.math')

// clang-format on

void RegisterVector2(sol::state& state) {
  // clang-format off

  /// Returns a vector with the minimum component-wise values of the inputs.
  // @function min
  // @param[type=Vector2|Vector3] v1
  // @param[type=Vector2|Vector3] v2
  // @treturn Vector2|Vector3
  // @usage assert(min(Vector2(0, 3), Vector2(1, 2)) == Vector2(0, 2))
  // assert(min(Vector3(0, 3, 2), Vector3(1, 2, 1)) == Vector3(0, 2, 1))
  state["min"] = sol::overload(&ovis::min<Vector2>, &ovis::min<Vector3>);

  /// Returns a vector with the maximum component-wise values of the inputs.
  // @function max
  // @param[type=Vector2|Vector3] v1
  // @param[type=Vector2|Vector3] v2
  // @treturn Vector2|Vector3
  // @usage assert(max(Vector2(0, 3), Vector2(1, 2)) == Vector2(1, 3))
  // assert(max(Vector3(0, 3, 2), Vector3(1, 2, 1)) == Vector3(1, 3, 2))
  state["max"] = sol::overload(&ovis::max<Vector2>, &ovis::max<Vector3>);

  /// Clamps the components of the vector to the specified range.
  // @function clamp
  // @param[type=Vector2|Vector3] v
  // @param[type=Vector2|Vector3] min
  // @param[type=Vector2|Vector3] max
  // @treturn Vector2|Vector3 The vector with the components clamped to the range [min, max]
  // @usage assert(clamp(Vector2(-1, 2), Vector2.ZERO, Vector2.ONE) == Vector2(0, 1))
  // assert(clamp(Vector3(-1, 2, 0.5), Vector3.ZERO, Vector3.ONE) == Vector3(0, 1, 0.5))
  state["clamp"] = sol::overload(&ovis::clamp<Vector2>, &ovis::clamp<Vector3>);

  /// Calculates the squared length of a vector.
  // This is faster than computing the actual length of the vector.
  // @function length_squared
  // @param[type=Vector2|Vector3] v
  // @treturn number The squared length of the vector
  // @usage assert(length_squared(Vector2(5, 5)) == 50)
  // assert(length_squared(Vector3(5, 5, 5)) == 75)
  state["length_squared"] = sol::overload(&ovis::SquaredLength<Vector2>, &ovis::SquaredLength<Vector3>);

  /// Calculates the length of a vector.
  // @function length
  // @param[type=Vector2|Vector3] v
  // @treturn number The length of the vector
  // @usage assert(length(Vector2(5, 5)) > 7.07106)
  // assert(length(Vector2(5, 5)) < 7.07107)
  // --
  // assert(length(Vector3(5, 5, 5)) > 8.6602)
  // assert(length(Vector3(5, 5, 5)) < 8.6603)
  state["length"] = sol::overload(&Length<Vector2>, &Length<Vector3>);

  /// Returns the normalized vector.
  // @function normalize
  // @param[type=Vector3] v
  // @treturn Vector3 Returns a vector with the same direction but with a length of 1
  // @usage assert(normalize(Vector2(5, 0)) == Vector2(1, 0))
  // assert(normalize(Vector3(5, 0, 0)) == Vector3(1, 0, 0))
  state["normalize"] = sol::overload(&Normalize<Vector2>, &Normalize<Vector3>);

  /// Calculates the dot product between two vectors.
  // The function is overloaded for both, @{Vector2} and @{Vector3}. However, both
  // inputs need to be of the same type.
  // @function dot
  // @see Vector2:__mul
  // @see Vector3:__mul
  // @see cross
  // @param[type=Vector2|Vector3] v1
  // @param[type=Vector2|Vector3] v2
  // @treturn number
  // @usage local v1 = Vector3(1, 2, 3)
  // local v2 = Vector3(4, 5, 6)
  // assert(dot(v1, v2) == 32)
  state["dot"] = sol::overload(
    &Dot<Vector2>,
    &Dot<Vector3>
  );

  /// Calculates the dot product between two vectorss.
  // @function cross
  // @see Vector3:__mul
  // @see dot
  // @param[type=Vector3] v1
  // @param[type=Vector3] v2
  // @treturn Vector3
  // @usage assert(cross(Vector3.POSITIVE_X, Vector3.POSITIVE_Y) == Vector3.POSITIVE_Z)
  state["cross"] = sol::overload(&Cross);


  /// A two-dimensional vector.
  // @type Vector2
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
  // @see dot
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
  // @see length
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
}

void RegisterColor(sol::state& state) {
  // clang-format off

  /// Represents a color usng red, green, blue and alpa.
  // The value for r, g, b and a range from 0 to 1. E.g., the RGB values
  // 0,0,0 respresent black and 1,1,1 respresent white. Similarly, an
  // alpha value of 0 represents full transparency whereas an alpha value
  // of 1 represents full opaqueness.
  // @type Color
  sol::usertype<Color> color_type = state.new_usertype<Color>(
    "Color", sol::constructors<Color(float, float, float), Color(float, float, float, float)>(),
    "new", sol::factories(
      [](const sol::table&) { return Color(0, 0, 0, 1.0f); },
      [](const sol::table&, float r, float g, float b) { return Color(r, g, b); },
      [](const sol::table&, float r, float g, float b, float a) { return Color(r, g, b, a); }
    ),
    sol::call_constructor, sol::factories(
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
  // @usage local c = Color:new()
  // assert(c.r == 0 and c.g == 0 and c.b == 0 and c.a == 1)

  /// Creates a new color from the arguments.
  // @function new
  // @param[type=number] r The value for the red channel.
  // @param[type=number] g The value for the green channel.
  // @param[type=number] b The value for the blue channel.
  // @param[type=number,opt=1] a The value for the alpha channel.
  // @treturn Color
  // @usage local c = Color:new(1.0, 0.0, 0.5)
  // assert(c.r == 1 and c.g == 0 and c.b == 0.5 and c.a == 1)

  /// Creates a new color.
  // Simplifies the syntax for creating colors.
  // @function __call
  // @treturn Color
  // @usage -- All the following calls create the same color:
  // local c1 = Color:new(1, 0.5, 1)
  // local c2 = Color(1, 0.5, 1)
  // local c3 = Color{1, 0.5, 1}
  // local c4 = Color{r = 1, g = 0.5, b = 1}
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
  // @usage local c1 = Color:new(1.0, 2.0, 3.0)
  // local c2 = Color:new(1.0, 2.0, 3.0)
  // local c3 = Color:new(1.0, 0.0, 3.0)
  // assert(c1 == c2) -- c1 and c2 are equal
  // assert(c1 ~= c3) -- c1 and c3 are not
  color_type[sol::meta_function::equal_to] = static_cast<bool (*)(const Color&, const Color&)>(ovis::operator==);

  /// Adds two colors.
  // @function __add
  // @param[type=Color] c1
  // @param[type=Color] c2
  // @treturn Color
  // @usage local c1 = Color:new(1, 0.5, 1)
  // local c2 = Color:new(0.5, 1, 0.5)
  // local c3 = c1 + c2
  // assert(c3 == Color:new(1.5, 1.5, 1.5, 2))
  color_type[sol::meta_function::addition] =
      static_cast<Color (*)(const Color&, const Color&)>(ovis::operator+);

  /// Negates a color.
  // Negative values for colors are not necessarily useful, however,
  // it make some algorithm more straightforward to implement.
  // @function __unm
  // @param[type=Color] c
  // @treturn Color
  // @usage local c1 = Color:new(1, -2, 3)
  // local c2 = -c1
  // assert(c2 == Color:new(-1, 2, -3, -1))
  color_type[sol::meta_function::unary_minus] = static_cast<Color (*)(const Color&)>(ovis::operator-);

  /// Subtracts two colors.
  // @function __sub
  // @param[type=Color] c1
  // @param[type=Color] c2
  // @treturn Color
  // @usage local c1 = Color:new(1, 0.5, 1)
  // local c2 = Color:new(0.5, 0.5, 0.5, 0)
  // local c3 = c1 - c2
  // assert(c3 == Color:new(0.5, 0, 0.5))
  color_type[sol::meta_function::subtraction] =
      static_cast<Color (*)(const Color&, const Color&)>(ovis::operator-);

  /// Multiplies two colors or a scalar and a color.
  // @function __mul
  // @param[type=Color|number] c1
  // @param[type=Color|number] c2
  // @treturn Color
  // @usage local c1 = Color:new(1, 0.5, 1)
  // local c2 = Color:new(0.5, 1, 0.5)
  // local c3 = c1 * c2 -- multiply two colors component-wise
  // assert(c3 == Color:new(0.5, 0.5, 0.5))
  // local v4 = c1 * 0.5 -- multiply a color and a scalar
  // assert(v4 == Color:new(0.5, 0.25, 0.5, 0.5))
  // local v5 = 0.5 * c1 -- you can also multiply from the other side
  // assert(v5 == Color:new(0.5, 0.25, 0.5, 0.5))
  color_type[sol::meta_function::multiplication] =
      sol::overload(static_cast<Color (*)(const Color&, const Color&)>(ovis::operator*),
                    static_cast<Color (*)(float, const Color&)>(ovis::operator*),
                    static_cast<Color (*)(const Color&, float)>(ovis::operator*));

  /// Divides two colors or a scalar and a color.
  // @function __div
  // @param[type=Color|number] c1
  // @param[type=Color|number] c2
  // @treturn Color
  // @usage local c1 = Color:new(1, 2, 4)
  // local c2 = Color:new(8, 4, 16)
  // local c3 = c1 / c2 -- divides two colors component-wise
  // assert(c3 == Color:new(0.125, 0.5, 0.25))
  // local v4 = c1 / 2 -- divies a color by a scalar
  // assert(v4 == Color:new(0.5, 1, 2, 0.5))
  // -- if you divide a scalar by a color a new color is created
  // -- by dividing the scalar by each component of the color
  // local v5 = 2 / c1
  // assert(v5 == Color:new(2, 1, 0.5, 2))
  color_type[sol::meta_function::division] =
      sol::overload(static_cast<Color (*)(const Color&, const Color&)>(ovis::operator/),
                    static_cast<Color (*)(float, const Color&)>(ovis::operator/),
                    static_cast<Color (*)(const Color&, float)>(ovis::operator/));

  /// Provides the length operator.
  // This returns the number of components in the color.
  // @function __len
  // @treturn number The number of compoenents in the color (4).
  // @usage local c = Color:new()
  // assert(#c == 4)
  color_type[sol::meta_function::length] = [](const Color& color) { return 4; };

  /// Provides string conversion.
  // @function __tostring
  // @treturn string The color components formatted as "(r, g, b, a)"
  // @usage local v = Color:new(1, 0.5, 1)
  // assert(tostring(v) == '(1.0, 0.5, 1.0, 1.0)')
  color_type[sol::meta_function::to_string] = [](const Color& color) { return fmt::format("{}", color); };

  // clang-format on
}

}  // namespace ovis
