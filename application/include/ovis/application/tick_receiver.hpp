#pragma once

#include <chrono>
#include "ovis/utils/all.hpp"

namespace ovis {

class TickReceiver : public All<TickReceiver> {
 public:
  virtual void Update(std::chrono::microseconds delta_time) = 0;
};

}  // namespace ovis
