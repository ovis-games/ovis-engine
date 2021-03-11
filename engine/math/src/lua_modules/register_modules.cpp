#include "module_loader.hpp"

#include <sol/sol.hpp>

#include <ovis/core/log.hpp>
#include <ovis/math/lua_modules/register_modules.hpp>
#include <ovis/math/vector.hpp>

namespace ovis {

void RegisterMathLuaModules(lua_State* l) {
  sol::state_view state(l);
  state.require("ovis.math.Vector2", LoadVector3Module);
  state.require("ovis.math.Vector3", LoadVector3Module);
  state.require("ovis.math.Color", LoadColorModule);
  state.require("ovis.math", LoadMathModule);
}

int LoadMathModule(lua_State* l) {
  sol::state_view state(l);

  /// This module contains math related types and function.
  // The types defined in this module are designed to behave as close to built-in
  // types as possible. E.g., they have all necessary basic math operators
  // defined in a meaningful way.
  // @module ovis.math
  // @usage -- require('ovis.math')
  sol::table math_module = state.create_table();

  math_module["Vector2"] = state["require"]("ovis.math.Vector2");
  math_module["Vector3"] = state["require"]("ovis.math.Vector3");
  math_module["Color"] = state["require"]("ovis.math.Color");

  // clang-format off

  /// Returns a vector with the minimum component-wise values of the inputs.
  // @function min
  // @param[type=Vector2|Vector3] v1
  // @param[type=Vector2|Vector3] v2
  // @treturn Vector2|Vector3
  // @usage assert(min(Vector2(0, 3), Vector2(1, 2)) == Vector2(0, 2))
  // assert(min(Vector3(0, 3, 2), Vector3(1, 2, 1)) == Vector3(0, 2, 1))
  math_module["min"] = sol::overload(&ovis::min<Vector2>, &ovis::min<Vector3>);

  /// Returns a vector with the maximum component-wise values of the inputs.
  // @function max
  // @param[type=Vector2|Vector3] v1
  // @param[type=Vector2|Vector3] v2
  // @treturn Vector2|Vector3
  // @usage assert(max(Vector2(0, 3), Vector2(1, 2)) == Vector2(1, 3))
  // assert(max(Vector3(0, 3, 2), Vector3(1, 2, 1)) == Vector3(1, 3, 2))
  math_module["max"] = sol::overload(&ovis::max<Vector2>, &ovis::max<Vector3>);

  /// Clamps the components of the vector to the specified range.
  // @function clamp
  // @param[type=Vector2|Vector3] v
  // @param[type=Vector2|Vector3] min
  // @param[type=Vector2|Vector3] max
  // @treturn Vector2|Vector3 The vector with the components clamped to the range [min, max]
  // @usage assert(clamp(Vector2(-1, 2), Vector2.ZERO, Vector2.ONE) == Vector2(0, 1))
  // assert(clamp(Vector3(-1, 2, 0.5), Vector3.ZERO, Vector3.ONE) == Vector3(0, 1, 0.5))
  math_module["clamp"] = sol::overload(&ovis::clamp<Vector2>, &ovis::clamp<Vector3>);

  /// Calculates the squared length of a vector.
  // This is faster than computing the actual length of the vector.
  // @function length_squared
  // @param[type=Vector2|Vector3] v
  // @treturn number The squared length of the vector
  // @usage assert(length_squared(Vector2(5, 5)) == 50)
  // assert(length_squared(Vector3(5, 5, 5)) == 75)
  math_module["length_squared"] = sol::overload(&ovis::SquaredLength<Vector2>, &ovis::SquaredLength<Vector3>);

  /// Calculates the length of a vector.
  // @function length
  // @param[type=Vector2|Vector3] v
  // @treturn number The length of the vector
  // @usage assert(length(Vector2(5, 5)) > 7.07106)
  // assert(length(Vector2(5, 5)) < 7.07107)
  // --
  // assert(length(Vector3(5, 5, 5)) > 8.6602)
  // assert(length(Vector3(5, 5, 5)) < 8.6603)
  math_module["length"] = sol::overload(&Length<Vector2>, &Length<Vector3>);

  /// Returns the normalized vector.
  // @function normalize
  // @param[type=Vector3] v
  // @treturn Vector3 Returns a vector with the same direction but with a length of 1
  // @usage assert(normalize(Vector2(5, 0)) == Vector2(1, 0))
  // assert(normalize(Vector3(5, 0, 0)) == Vector3(1, 0, 0))
  math_module["normalize"] = sol::overload(&Normalize<Vector2>, &Normalize<Vector3>);

  /// Calculates the dot product between two vectors.
  // The function is overloaded for both, @{Vector2} and @{Vector3}. However, both
  // inputs need to be of the same type.
  // @function dot
  // @see cross
  // @param[type=ovis.math.Vector2|ovis.math.Vector3] v1
  // @param[type=ovis.math.Vector2|ovis.math.Vector3] v2
  // @treturn number
  // @usage local v1 = Vector3(1, 2, 3)
  // local v2 = Vector3(4, 5, 6)
  // assert(dot(v1, v2) == 32)
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
  // @usage assert(cross(Vector3.POSITIVE_X, Vector3.POSITIVE_Y) == Vector3.POSITIVE_Z)
  math_module["cross"] = sol::overload(&Cross);

  return math_module.push();
}

}  // namespace ovis