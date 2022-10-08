#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

namespace ovis {

template <typename T>
class All {
 public:
  All() { all_.push_back(static_cast<T*>(this)); }
  All(const All<T>& other) : All() {}
  All(All<T>&& other) : All() {}
  ~All() {
    const auto it = std::find(all_.begin(), all_.end(), static_cast<T*>(this));
    assert(it != all_.end());
    all_.erase(it);
  }

  static const std::vector<T*> all() { return all_; }

 private:
  static std::vector<T*> all_;
};

template <typename T> std::vector<T*> All<T>::all_;

}  // namespace ovis
