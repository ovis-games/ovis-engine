#pragma once

#include <type_traits>
#include <vector>

namespace ovis {

class LuaReferenceBase {};

template <typename T>
class LuaReference;

class LuaReferencable {
 public:
  virtual ~LuaReferencable() {}

 private:
  std::vector<LuaReferenceBase*> references_;
};

template <typename T>
class LuaReference {
  static_assert(std::is_base_of_v<LuaReferencable, T>());

 public:
  LuaReference() {}

  inline const T* operator->() const { return value_; }
  inline T* operator->() { return value_; }

  inline const T& operator*() const { return *value_; }
  inline T& operator*() { return *value_; }

 private:
  T* value_;
};

}  // namespace ovis
