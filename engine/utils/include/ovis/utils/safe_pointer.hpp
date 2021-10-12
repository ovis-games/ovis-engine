#pragma once

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <sol/sol.hpp>

#include <ovis/utils/log.hpp>

namespace ovis {

class safe_ptr_base;

class SafelyReferenceable {
  friend class safe_ptr_base;

 protected:
  inline SafelyReferenceable();
  inline SafelyReferenceable(const SafelyReferenceable& other);
  inline SafelyReferenceable(SafelyReferenceable&& other);
  inline ~SafelyReferenceable();

  inline SafelyReferenceable& operator=(const SafelyReferenceable& other);
  inline SafelyReferenceable& operator=(SafelyReferenceable&& other);

 private:
  std::vector<safe_ptr_base*> references_;
};

class safe_ptr_base {
  friend class SafelyReferenceable;

 protected:
  inline constexpr safe_ptr_base() noexcept : pointer_{nullptr} {}
  inline constexpr safe_ptr_base(SafelyReferenceable* pointer) : pointer_{pointer} {
    if (pointer_) {
      pointer_->references_.push_back(this);
    }
  }
  inline constexpr safe_ptr_base(const safe_ptr_base& other) : pointer_{other.pointer_} {
    if (pointer_) {
      pointer_->references_.push_back(this);
    }
  }
  inline constexpr safe_ptr_base(safe_ptr_base&& other) : pointer_{other.pointer_} {
    if (pointer_) {
      auto reference_iterator = std::find(pointer_->references_.begin(), pointer_->references_.end(), &other);
      assert(reference_iterator != pointer_->references_.end());
      *reference_iterator = this;
      other.pointer_ = nullptr;
    }
  }
  inline ~safe_ptr_base() { reset(); }

  inline constexpr safe_ptr_base& operator=(const safe_ptr_base& other) {
    reset(other.pointer_);
    return *this;
  }

  inline constexpr safe_ptr_base& operator=(safe_ptr_base&& other) {
    reset(other.pointer_);
    other.reset();
    return *this;
  }

  inline constexpr void reset(SafelyReferenceable* new_value = nullptr) {
    if (pointer_) {
      auto& references = pointer_->references_;
      auto reference_iterator = std::find(references.begin(), references.end(), this);
      assert(reference_iterator != references.end());
      if (references.size() > 1) {
        *reference_iterator = references.back();
      }
      references.pop_back();
    }
    pointer_ = new_value;
    if (pointer_) {
      pointer_->references_.push_back(this);
    }
  }

  SafelyReferenceable* pointer_;
};

template <typename T>
concept SafelyReferenceableObject = std::is_base_of_v<SafelyReferenceable, T>;

template <typename T>
class safe_ptr : public safe_ptr_base {
  // T must derive from SafelyReferenceable, however, the static_assert below
  // prevents the use of safe_ptr<T> as a member variable of T, because
  // std::is_base_of_v requires that the type T is fully defined which it is
  // not in that case. Without the assert it works properly but you will get
  // a more cryptic error message when using this class with the wrong type.
  // static_assert(std::is_base_of_v<SafelyReferenceable, T>,
  //               "The pointer type must derive from ovis::SafelyReferenceable");

 public:
  inline constexpr safe_ptr() noexcept {}
  inline constexpr safe_ptr(std::nullptr_t) noexcept {}
  inline explicit safe_ptr(T* pointer) : safe_ptr_base(pointer) {}

  inline safe_ptr<T>& operator=(T* other) {
    reset(other);
    return *this;
  }

  inline T* get() const noexcept { return static_cast<T*>(pointer_); }
  inline T* get_throw() const {
    if (pointer_ == nullptr) {
      throw std::runtime_error("Trying to access non valid pointer.");
    }
    return get();
  }
  inline T& operator*() const { return *get_throw(); }
  inline T* operator->() const { return get_throw(); }
  inline operator bool() const { return get() != nullptr; }

  inline void reset(T* new_value = nullptr) { safe_ptr_base::reset(new_value); }
};

template <typename T>
inline bool operator==(const safe_ptr<T>& lhs, const safe_ptr<T>& rhs) {
  return lhs.get() == rhs.get();
}

template <typename T>
inline bool operator==(const safe_ptr<T>& lhs, T* const rhs) {
  return lhs.get() == rhs;
}

template <typename T>
inline bool operator==(T* const lhs, const safe_ptr<T> rhs) {
  return lhs == rhs.get();
}

template <typename T>
inline bool operator==(const safe_ptr<T>& lhs, std::nullptr_t) {
  return lhs.get() == nullptr;
}

template <typename T>
inline bool operator==(std::nullptr_t, const safe_ptr<T>& rhs) {
  return nullptr == rhs.get();
}

template <typename T>
safe_ptr(T* pointer) -> safe_ptr<T>;

inline SafelyReferenceable::SafelyReferenceable() {}
inline SafelyReferenceable::SafelyReferenceable(const SafelyReferenceable& other) {}
inline SafelyReferenceable::SafelyReferenceable(SafelyReferenceable&& other) {
  std::swap(references_, other.references_);
  for (safe_ptr_base* reference : references_) {
    reference->pointer_ = this;
  }
}
inline SafelyReferenceable::~SafelyReferenceable() {
  for (safe_ptr_base* reference : references_) {
    reference->pointer_ = nullptr;
  }
}

inline SafelyReferenceable& SafelyReferenceable::operator=(const SafelyReferenceable& other) {
  return *this;
}
inline SafelyReferenceable& SafelyReferenceable::operator=(SafelyReferenceable&& other) {
  for (safe_ptr_base* reference : other.references_) {
    reference->pointer_ = this;
  }
  references_.insert(references_.end(), other.references_.begin(), other.references_.end());
  other.references_.clear();
  return *this;
}

}  // namespace ovis

namespace sol {
template <typename T>
struct unique_usertype_traits<ovis::safe_ptr<T>> {
  typedef T type;
  typedef ovis::safe_ptr<T> actual_type;
  static const bool value = true;

  static bool is_null(const actual_type& ptr) { return ptr == nullptr; }

  static type* get(const actual_type& ptr) { return ptr.get(); }
};
}  // namespace sol

template <typename T, std::enable_if_t<std::is_base_of_v<ovis::SafelyReferenceable, T>, bool> = true>
void sol_lua_check_access(sol::types<T>, lua_State* L, int index, sol::stack::record& tracking) {
  sol::optional<ovis::safe_ptr<T>&> safe_ptr =
      sol::stack::check_get<ovis::safe_ptr<T>&>(L, index, sol::no_panic, tracking);
  if (!safe_ptr.has_value()) {
    return;
  }

  if ((*safe_ptr).get() == nullptr) {
    // freak out in whatever way is appropriate, here
    throw std::runtime_error("You dun goofed");
  }
}
