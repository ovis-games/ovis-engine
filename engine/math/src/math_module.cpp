#include <sol/sol.hpp>

#include <ovis/core/core_module.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/math/math_module.hpp>
#include <ovis/math/vector.hpp>

namespace ovis {

namespace {

int LoadMathModule(lua_State* l) {
  sol::state_view state(l);

  /// This module contains math related types and function.
  // The types defined in this module are designed to behave as close to built-in
  // types as possible. E.g., they have all necessary basic math operators
  // defined in a meaningful way.
  // @module ovis.math
  // @usage local math = require "ovis.math"
  // local Vector2 = math.Vector2
  // local Vector3 = math.Vector3
  // local Color = math.Color
  sol::table math_module = state.create_table();

  // Register types
  math_module["abs"] = state["math"]["abs"];
  math_module["acos"] = state["math"]["abs"];
  math_module["asin"] = state["math"]["asin"];
  math_module["atan"] = state["math"]["atan"];
  math_module["ceil"] = state["math"]["ceil"];
  math_module["cos"] = state["math"]["cos"];
  math_module["deg"] = state["math"]["deg"];
  math_module["exp"] = state["math"]["exp"];
  math_module["floor"] = state["math"]["floor"];
  math_module["fmod"] = state["math"]["fmod"];
  math_module["huge"] = state["math"]["huge"];
  math_module["log"] = state["math"]["log"];
  math_module["max"] = state["math"]["max"];
  math_module["maxinteger"] = state["math"]["maxinteger"];
  math_module["min"] = state["math"]["min"];
  math_module["mininteger"] = state["math"]["mininteger"];
  math_module["modf"] = state["math"]["modf"];
  math_module["pi"] = state["math"]["pi"];
  math_module["rad"] = state["math"]["rad"];
  math_module["random"] = state["math"]["random"];
  math_module["randomseed"] = state["math"]["randomseed"];
  math_module["sin"] = state["math"]["sin"];
  math_module["sqrt"] = state["math"]["sqrt"];
  math_module["tointeger"] = state["math"]["tointeger"];
  math_module["type"] = state["math"]["type"];
  math_module["ult"] = state["math"]["ult"];

  // clang-format off

  /// Returns a vector with the minimum component-wise values of the inputs.
  // @function min
  // @param[type=Vector2|Vector3] v1
  // @param[type=Vector2|Vector3] v2
  // @treturn Vector2|Vector3
  // @usage assert(math.min(Vector2.new(0, 3), Vector2.new(1, 2)) == Vector2.new(0, 2))
  // assert(math.min(Vector3.new(0, 3, 2), Vector3.new(1, 2, 1)) == Vector3.new(0, 2, 1))
  math_module["min"] = sol::overload(&ovis::min<Vector2>, &ovis::min<Vector3>);

  /// Returns a vector with the maximum component-wise values of the inputs.
  // @function max
  // @param[type=Vector2|Vector3] v1
  // @param[type=Vector2|Vector3] v2
  // @treturn Vector2|Vector3
  // @usage assert(math.max(Vector2.new(0, 3), Vector2.new(1, 2)) == Vector2.new(1, 3))
  // assert(math.max(Vector3.new(0, 3, 2), Vector3.new(1, 2, 1)) == Vector3.new(1, 3, 2))
  math_module["max"] = sol::overload(&ovis::max<Vector2>, &ovis::max<Vector3>);

  /// Clamps the components of the vector to the specified range.
  // @function clamp
  // @param[type=Vector2|Vector3] v
  // @param[type=Vector2|Vector3] min
  // @param[type=Vector2|Vector3] max
  // @treturn Vector2|Vector3 The vector with the components clamped to the range [min, max]
  // @usage assert(math.clamp(Vector2.new(-1, 2), Vector2.ZERO, Vector2.ONE) == Vector2.new(0, 1))
  // assert(math.clamp(Vector3.new(-1, 2, 0.5), Vector3.ZERO, Vector3.ONE) == Vector3.new(0, 1, 0.5))
  math_module["clamp"] = sol::overload(&ovis::clamp<Vector2>, &ovis::clamp<Vector3>);

  /// Calculates the squared length of a vector.
  // This is faster than computing the actual length of the vector.
  // @function length_squared
  // @param[type=Vector2|Vector3] v
  // @treturn number The squared length of the vector
  // @usage assert(math.length_squared(Vector2.new(5, 5)) == 50)
  // assert(math.length_squared(Vector3.new(5, 5, 5)) == 75)
  math_module["length_squared"] = sol::overload(&ovis::SquaredLength<Vector2>, &ovis::SquaredLength<Vector3>);

  /// Calculates the length of a vector.
  // @function length
  // @param[type=Vector2|Vector3] v
  // @treturn number The length of the vector
  // @usage assert(math.length(Vector2.new(5, 5)) > 7.07106)
  // assert(math.length(Vector2.new(5, 5)) < 7.07107)
  //
  // assert(math.length(Vector3.new(5, 5, 5)) > 8.6602)
  // assert(math.length(Vector3.new(5, 5, 5)) < 8.6603)
  math_module["length"] = sol::overload(&Length<Vector2>, &Length<Vector3>);

  /// Returns the normalized vector.
  // @function normalize
  // @param[type=Vector3] v
  // @treturn Vector3 Returns a vector with the same direction but with a length of 1
  // @usage assert(math.normalize(Vector2.new(5, 0)) == Vector2.new(1, 0))
  // assert(math.normalize(Vector3.new(5, 0, 0)) == Vector3.new(1, 0, 0))
  math_module["normalize"] = sol::overload(&Normalize<Vector2>, &Normalize<Vector3>);

  /// Calculates the dot product between two vectors.
  // The function is overloaded for both, @{Vector2} and @{Vector3}. However, both
  // inputs need to be of the same type.
  // @function dot
  // @see cross
  // @param[type=ovis.math.Vector2|ovis.math.Vector3] v1
  // @param[type=ovis.math.Vector2|ovis.math.Vector3] v2
  // @treturn number
  // @usage local v1 = Vector3.new(1, 2, 3)
  // local v2 = Vector3.new(4, 5, 6)
  // assert(math.dot(v1, v2) == 32)
  math_module["dot"] = sol::overload(
    &Dot<Vector2>,
    &Dot<Vector3>
  );

  /// Calculates the dot product between two vectorss.
  // @function cross
  // @see dot
  // @param[type=ovis.math.Vector3] v1
  // @param[type=ovis.math.Vector3] v2
  // @treturn Vector3
  // @usage assert(math.cross(Vector3.POSITIVE_X, Vector3.POSITIVE_Y) == Vector3.POSITIVE_Z)
  math_module["cross"] = sol::overload(&Cross);

  return math_module.push();
}

}

bool LoadMathModule() {
  static bool module_loaded = false;
  if (!module_loaded) {
    LoadCoreModule();
    lua.require("ovis.math", &LoadMathModule);
    module_loaded = true;
  }

  return true;
}

}  // namespace ovis