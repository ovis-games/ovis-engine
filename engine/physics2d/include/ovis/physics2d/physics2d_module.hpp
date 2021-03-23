#pragma once

#include <ovis/engine/module.hpp>

namespace ovis {

class Physics2DModule final : public Module {
 public:
  Physics2DModule();
  ~Physics2DModule() override;
};

}  // namespace ovis