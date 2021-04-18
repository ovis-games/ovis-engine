#pragma once

#include <type_traits>
#include <vector>

#include <sol/sol.hpp>

#include <ovis/utils/safe_pointer.hpp>

namespace ovis {

// class LuaReferenceBase {};

// template <typename T>
// class LuaReference;

class DynamicallyLuaReferencableBase : public SafelyReferenceable {
 public:
  virtual ~DynamicallyLuaReferencableBase() = default;

  virtual sol::lua_value GetValue() = 0;
};

// Makes classes dyamically lua referencable, e.g., it takes into account the most derived type
// when passing values to lua.
template <typename T>
class DynamicallyLuaReferencable : public DynamicallyLuaReferencableBase {
 public:
  virtual ~DynamicallyLuaReferencable() {}

  sol::lua_value GetValue() override { return safe_ptr<T>(this); }
};

#define OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE() \
  sol::lua_value GetValue() override { return safe_ptr(this); }

// template <typename T>
// class LuaReference {
//   static_assert(std::is_base_of_v<LuaReferencable, T>());

//  public:
//   LuaReference() {}

//   inline const T* operator->() const { return value_; }
//   inline T* operator->() { return value_; }

//   inline const T& operator*() const { return *value_; }
//   inline T& operator*() { return *value_; }

//  private:
//   T* value_;
// };

}  // namespace ovis
