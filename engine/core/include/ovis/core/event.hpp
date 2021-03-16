#pragma once

#include <string>
#include <string_view>

namespace ovis {

class Event {
 public:
  inline Event(std::string type) : type_(std::move(type)) {}
  virtual ~Event() = default;

  inline std::string_view type() const { return type_; }

  inline bool is_propagating() const { return is_propagating_; }
  inline void StopPropagation() { is_propagating_ = false; }

 private:
  std::string type_;
  bool is_propagating_ = true;
};

}  // namespace ovis
